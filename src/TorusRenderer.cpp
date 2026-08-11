#ifndef __EMSCRIPTEN__

#include "TorusRenderer.hpp"
#include "RenderConstants.hpp"
#include "Shader.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <cmath>

namespace {

// Subdivisions per lattice edge. An edge on a torus is an arc, not a chord --
// without subdivision the ribbons would visibly cut through the surface.
constexpr int EDGE_SEGMENTS = 10;

// Surface tessellation. Independent of the lattice: this is just the skin.
constexpr int SURFACE_MAJOR_STEPS = 96;
constexpr int SURFACE_MINOR_STEPS = 40;

// The flat view's half-widths (line.cpp) are tuned to its ~1.8-unit span. The
// torus is several times larger in world units, so edges need scaling up to
// read at the same relative thickness.
constexpr float EDGE_WIDTH_SCALE = 7.0f;

// Lift ribbons and markers clear of the skin so they don't z-fight with it.
constexpr float EDGE_SURFACE_OFFSET   = 0.012f;
constexpr float MARKER_SURFACE_OFFSET = 0.030f;

const char* VERTEX_SRC = R"GLSL(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
uniform mat4 u_viewProjection;
uniform float u_pointSize;
out vec3 vNormal;
void main() {
    gl_Position = u_viewProjection * vec4(aPos, 1.0);
    gl_PointSize = u_pointSize;
    vNormal = aNormal;
}
)GLSL";

// u_isPoint turns on a circular mask via gl_PointCoord, so markers render as
// actual discs instead of squares -- matching the flat view's circles without
// needing a separate disc mesh oriented to the surface at every site.
const char* FRAGMENT_SRC = R"GLSL(
#version 330 core
in vec3 vNormal;
out vec4 FragColor;
uniform vec4 u_color;
uniform int u_isPoint;
uniform int u_shaded;
void main() {
    if (u_isPoint == 1) {
        vec2 offset = gl_PointCoord - vec2(0.5);
        if (dot(offset, offset) > 0.25) discard;
    }
    float light = 1.0;
    if (u_shaded == 1) {
        vec3 lightDir = normalize(vec3(0.35, 0.45, 0.82));
        light = 0.62 + 0.38 * max(dot(normalize(vNormal), lightDir), 0.0);
    }
    FragColor = vec4(u_color.rgb * light, u_color.a);
}
)GLSL";

void pushVertex(std::vector<float>& out, const glm::vec3& p, const glm::vec3& n) {
    out.insert(out.end(), { p.x, p.y, p.z, n.x, n.y, n.z });
}

void configurePositionNormalAttribs() {
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

} // namespace

void TorusRenderer::initialize(const Lattice& lattice) {
    torusMap.period = (float)lattice.dimension();
    program = Shader::createProgramFromSource(VERTEX_SRC, FRAGMENT_SRC);

    buildSurface();
    buildDots(lattice);

    // --- edge ribbons ------------------------------------------------------
    const size_t quadsPerEdge = EDGE_SEGMENTS;
    const size_t floatsPerEdge = quadsPerEdge * 4 * 6;
    edgeVertices.assign(lattice.edgeCount() * floatsPerEdge, 0.0f);

    glGenVertexArrays(1, &edgeVAO);
    glBindVertexArray(edgeVAO);
    glGenBuffers(1, &edgeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, edgeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * edgeVertices.size(), nullptr, GL_DYNAMIC_DRAW);
    configurePositionNormalAttribs();

    std::vector<unsigned int> indices;
    const size_t totalQuads = lattice.edgeCount() * quadsPerEdge;
    indices.reserve(totalQuads * 6);
    for (size_t q = 0; q < totalQuads; q++) {
        unsigned int base = static_cast<unsigned int>(q * 4);
        indices.insert(indices.end(), { base + 0, base + 1, base + 2,
                                        base + 0, base + 2, base + 3 });
    }
    glGenBuffers(1, &edgeEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, edgeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(),
                 indices.data(), GL_STATIC_DRAW);

    // --- defect markers ----------------------------------------------------
    const size_t maxMarkers = lattice.plaquetteCount() + lattice.vertexCount();
    glGenVertexArrays(1, &defectVAO);
    glBindVertexArray(defectVAO);
    glGenBuffers(1, &defectVBO);
    glBindBuffer(GL_ARRAY_BUFFER, defectVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * maxMarkers * 6, nullptr, GL_DYNAMIC_DRAW);
    configurePositionNormalAttribs();

    sync(lattice);
}

void TorusRenderer::buildSurface() {
    std::vector<float> data;
    std::vector<unsigned int> indices;

    // Vertex grid is (steps+1) x (steps+1) so the seam row/column duplicates
    // position but keeps index math trivial.
    for (int i = 0; i <= SURFACE_MAJOR_STEPS; i++) {
        float u = torusMap.period * (float)i / SURFACE_MAJOR_STEPS;
        for (int j = 0; j <= SURFACE_MINOR_STEPS; j++) {
            float v = torusMap.period * (float)j / SURFACE_MINOR_STEPS;
            pushVertex(data, torusMap.position(u, v), torusMap.normal(u, v));
        }
    }

    const int stride = SURFACE_MINOR_STEPS + 1;
    for (int i = 0; i < SURFACE_MAJOR_STEPS; i++) {
        for (int j = 0; j < SURFACE_MINOR_STEPS; j++) {
            unsigned int a = (unsigned int)(i * stride + j);
            unsigned int b = (unsigned int)((i + 1) * stride + j);
            unsigned int c = (unsigned int)((i + 1) * stride + j + 1);
            unsigned int d = (unsigned int)(i * stride + j + 1);
            indices.insert(indices.end(), { a, b, c, a, c, d });
        }
    }
    surfaceIndexCount = indices.size();

    glGenVertexArrays(1, &surfaceVAO);
    glBindVertexArray(surfaceVAO);
    glGenBuffers(1, &surfaceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, surfaceVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * data.size(), data.data(), GL_STATIC_DRAW);
    configurePositionNormalAttribs();

    glGenBuffers(1, &surfaceEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, surfaceEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(),
                 indices.data(), GL_STATIC_DRAW);
}

void TorusRenderer::buildDots(const Lattice& lattice) {
    const size_t dim = lattice.dimension();
    std::vector<float> data;

    // Only 0..dim-1: on the torus there is no seam to duplicate.
    for (size_t row = 0; row < dim; row++) {
        for (size_t col = 0; col < dim; col++) {
            float u = (float)col, v = (float)row;
            pushVertex(data, torusMap.position(u, v, MARKER_SURFACE_OFFSET * 0.5f),
                       torusMap.normal(u, v));
        }
    }
    dotCount = data.size() / 6;

    glGenVertexArrays(1, &dotVAO);
    glBindVertexArray(dotVAO);
    glGenBuffers(1, &dotVBO);
    glBindBuffer(GL_ARRAY_BUFFER, dotVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * data.size(), data.data(), GL_STATIC_DRAW);
    configurePositionNormalAttribs();
}

void TorusRenderer::syncEdges(const Lattice& lattice) {
    const size_t floatsPerEdge = (size_t)EDGE_SEGMENTS * 4 * 6;

    for (size_t logical = 0; logical < lattice.edgeCount(); logical++) {
        float u0, v0, u1, v1;
        lattice.edgeGridSpan(logical, u0, v0, u1, v1);
        const float halfWidth = lattice.edge(logical).getWidth() * EDGE_WIDTH_SCALE;
        const size_t base = logical * floatsPerEdge;

        for (int s = 0; s < EDGE_SEGMENTS; s++) {
            const float t0 = (float)s / EDGE_SEGMENTS;
            const float t1 = (float)(s + 1) / EDGE_SEGMENTS;

            const float ua = u0 + (u1 - u0) * t0, va = v0 + (v1 - v0) * t0;
            const float ub = u0 + (u1 - u0) * t1, vb = v0 + (v1 - v0) * t1;

            const glm::vec3 pA = torusMap.position(ua, va, EDGE_SURFACE_OFFSET);
            const glm::vec3 pB = torusMap.position(ub, vb, EDGE_SURFACE_OFFSET);
            const glm::vec3 nA = torusMap.normal(ua, va);
            const glm::vec3 nB = torusMap.normal(ub, vb);

            // Same quad construction as the flat view, except "perpendicular"
            // means perpendicular WITHIN the tangent plane: cross(normal,
            // direction). That keeps the ribbon flat against the surface
            // instead of tilting off it.
            const glm::vec3 direction = glm::normalize(pB - pA);
            const glm::vec3 perpA = glm::normalize(glm::cross(nA, direction)) * halfWidth;
            const glm::vec3 perpB = glm::normalize(glm::cross(nB, direction)) * halfWidth;

            const glm::vec3 corners[4] = { pA + perpA, pA - perpA, pB - perpB, pB + perpB };
            const glm::vec3 normals[4] = { nA, nA, nB, nB };

            size_t offset = base + (size_t)s * 4 * 6;
            for (int c = 0; c < 4; c++) {
                edgeVertices[offset + 0] = corners[c].x;
                edgeVertices[offset + 1] = corners[c].y;
                edgeVertices[offset + 2] = corners[c].z;
                edgeVertices[offset + 3] = normals[c].x;
                edgeVertices[offset + 4] = normals[c].y;
                edgeVertices[offset + 5] = normals[c].z;
                offset += 6;
            }
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, edgeVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * edgeVertices.size(), edgeVertices.data());
}

void TorusRenderer::syncDefects(const Lattice& lattice) {
    const size_t dim = lattice.dimension();
    std::vector<float> data;

    for (size_t row = 0; row < dim; row++) {
        for (size_t col = 0; col < dim; col++) {
            if (lattice.plaquetteFires(col, row)) {
                float u = col + 0.5f, v = row + 0.5f;
                pushVertex(data, torusMap.position(u, v, MARKER_SURFACE_OFFSET),
                           torusMap.normal(u, v));
            }
        }
    }
    plaquetteDefectPoints = data.size() / 6;

    for (size_t row = 0; row < dim; row++) {
        for (size_t col = 0; col < dim; col++) {
            if (lattice.vertexFires(col, row)) {
                float u = (float)col, v = (float)row;
                pushVertex(data, torusMap.position(u, v, MARKER_SURFACE_OFFSET),
                           torusMap.normal(u, v));
            }
        }
    }
    vertexDefectPoints = data.size() / 6 - plaquetteDefectPoints;

    if (!data.empty()) {
        glBindBuffer(GL_ARRAY_BUFFER, defectVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * data.size(), data.data());
    }
}

void TorusRenderer::sync(const Lattice& lattice) {
    syncEdges(lattice);
    syncDefects(lattice);
}

void TorusRenderer::draw(const Lattice& lattice, const OrbitCamera& camera,
                          int framebufferWidth, int framebufferHeight) {
    glUseProgram(program);

    const glm::mat4 viewProjection = camera.viewProjection(framebufferWidth, framebufferHeight);
    glUniformMatrix4fv(glGetUniformLocation(program, "u_viewProjection"), 1, GL_FALSE,
                       glm::value_ptr(viewProjection));

    const int colorLocation = glGetUniformLocation(program, "u_color");
    const int pointLocation = glGetUniformLocation(program, "u_isPoint");
    const int shadedLocation = glGetUniformLocation(program, "u_shaded");
    const int pointSizeLocation = glGetUniformLocation(program, "u_pointSize");

    auto setColor = [&](const glm::vec4& c) {
        glUniform4f(colorLocation, c.r, c.g, c.b, c.a);
    };

    // --- surface -----------------------------------------------------------
    glUniform1i(pointLocation, 0);
    glUniform1i(shadedLocation, 1);
    glUniform1f(pointSizeLocation, 1.0f);
    setColor(RenderConstants::TORUS_SURFACE_COLOR);
    glBindVertexArray(surfaceVAO);
    glDrawElements(GL_TRIANGLES, (GLsizei)surfaceIndexCount, GL_UNSIGNED_INT, 0);

    // --- edges -------------------------------------------------------------
    // Colors come from the same LineState lookup the flat window uses, so the
    // two views are guaranteed to agree.
    glBindVertexArray(edgeVAO);
    for (size_t logical = 0; logical < lattice.edgeCount(); logical++) {
        glm::vec4 color = lattice.edge(logical).getColor();
        if (color.a <= 0.0f) continue;   // fully transparent states are simply not drawn
        setColor(color);
        glDrawElements(GL_TRIANGLES, EDGE_SEGMENTS * 6, GL_UNSIGNED_INT,
                       (const void*)(sizeof(unsigned int) * EDGE_SEGMENTS * 6 * logical));
    }

    // --- lattice sites and defects (round point sprites) -------------------
    glUniform1i(pointLocation, 1);
    glUniform1i(shadedLocation, 0);

    glBindVertexArray(dotVAO);
    glUniform1f(pointSizeLocation, RenderConstants::TORUS_LATTICE_POINT_SIZE);
    setColor(RenderConstants::TORUS_LATTICE_DOT_COLOR);
    glDrawArrays(GL_POINTS, 0, (GLsizei)dotCount);

    glBindVertexArray(defectVAO);
    if (plaquetteDefectPoints > 0) {
        glUniform1f(pointSizeLocation, RenderConstants::TORUS_PLAQUETTE_POINT_SIZE);
        setColor(RenderConstants::PLAQUETTE_DEFECT_COLOR);
        glDrawArrays(GL_POINTS, 0, (GLsizei)plaquetteDefectPoints);
    }
    if (vertexDefectPoints > 0) {
        glUniform1f(pointSizeLocation, RenderConstants::TORUS_VERTEX_POINT_SIZE);
        setColor(RenderConstants::VERTEX_DEFECT_COLOR);
        glDrawArrays(GL_POINTS, (GLint)plaquetteDefectPoints, (GLsizei)vertexDefectPoints);
    }
}

void TorusRenderer::shutdown() {
    glDeleteBuffers(1, &surfaceVBO);
    glDeleteBuffers(1, &surfaceEBO);
    glDeleteBuffers(1, &edgeVBO);
    glDeleteBuffers(1, &edgeEBO);
    glDeleteBuffers(1, &dotVBO);
    glDeleteBuffers(1, &defectVBO);
    glDeleteVertexArrays(1, &surfaceVAO);
    glDeleteVertexArrays(1, &edgeVAO);
    glDeleteVertexArrays(1, &dotVAO);
    glDeleteVertexArrays(1, &defectVAO);
    glDeleteProgram(program);
}

#endif // __EMSCRIPTEN__
