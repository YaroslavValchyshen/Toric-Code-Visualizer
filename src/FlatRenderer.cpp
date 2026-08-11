#include "FlatRenderer.hpp"
#include "RenderConstants.hpp"

#include <glm/glm.hpp>
#include <cmath>

namespace {

// Appends a filled circle as a fan of independent triangles. Flat triangle
// soup (rather than GL_TRIANGLE_FAN) so many circles can live in one buffer
// and be drawn with a single call.
void appendCircle(std::vector<float>& out, glm::vec2 center, float radius) {
    for (int s = 0; s < RenderConstants::CIRCLE_SEGMENTS; s++) {
        float t0 = RenderConstants::TWO_PI * (float)s / RenderConstants::CIRCLE_SEGMENTS;
        float t1 = RenderConstants::TWO_PI * (float)(s + 1) / RenderConstants::CIRCLE_SEGMENTS;
        glm::vec2 p1 = center + radius * glm::vec2(std::cos(t0), std::sin(t0));
        glm::vec2 p2 = center + radius * glm::vec2(std::cos(t1), std::sin(t1));
        out.insert(out.end(), { center.x, center.y, p1.x, p1.y, p2.x, p2.y });
    }
}

} // namespace

void FlatRenderer::initialize(const Lattice& lattice) {
    const size_t dim = lattice.dimension();

    // --- lattice points ----------------------------------------------------
    std::vector<float> dots;
    for (size_t row = 0; row <= dim; row++) {
        for (size_t col = 0; col <= dim; col++) {
            glm::vec2 p = lattice.vertexPosition(col, row);
            dots.push_back(p.x);
            dots.push_back(p.y);
        }
    }
    dotCount = dots.size() / 2;

    glGenVertexArrays(1, &dotVAO);
    glBindVertexArray(dotVAO);
    glGenBuffers(1, &dotVBO);
    glBindBuffer(GL_ARRAY_BUFFER, dotVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * dots.size(), dots.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    // --- edge quads --------------------------------------------------------
    const size_t drawnEdges = lattice.drawnEdgeCount();
    edgeVertices.assign(drawnEdges * 8, 0.0f);

    glGenVertexArrays(1, &edgeVAO);
    glBindVertexArray(edgeVAO);
    glGenBuffers(1, &edgeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, edgeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * edgeVertices.size(), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    std::vector<unsigned int> indices;
    indices.reserve(drawnEdges * 6);
    for (size_t i = 0; i < drawnEdges; i++) {
        unsigned int base = static_cast<unsigned int>(i * 4);
        indices.insert(indices.end(), { base + 0, base + 1, base + 2,
                                        base + 0, base + 2, base + 3 });
    }
    glGenBuffers(1, &edgeEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, edgeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(),
                 indices.data(), GL_STATIC_DRAW);

    // --- defect markers ----------------------------------------------------
    // Sized for the worst case: every plaquette and every drawn grid point
    // firing at once, so sync() only ever needs glBufferSubData.
    const size_t maxCircles = lattice.plaquetteCount() + (dim + 1) * (dim + 1);
    glGenVertexArrays(1, &defectVAO);
    glBindVertexArray(defectVAO);
    glGenBuffers(1, &defectVBO);
    glBindBuffer(GL_ARRAY_BUFFER, defectVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(float) * maxCircles * RenderConstants::CIRCLE_SEGMENTS * 6,
                 nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    sync(lattice);
}

void FlatRenderer::sync(const Lattice& lattice) {
    syncEdges(lattice);
    syncDefects(lattice);
}

void FlatRenderer::syncEdges(const Lattice& lattice) {
    const std::vector<glm::vec2>& endpoints = lattice.drawnEndpoints();

    for (size_t drawn = 0; drawn < lattice.drawnEdgeCount(); drawn++) {
        glm::vec2 A = endpoints[drawn * 2 + 0];
        glm::vec2 B = endpoints[drawn * 2 + 1];

        // Width comes from the LOGICAL edge, so both drawn copies of a seam
        // edge thicken together.
        float halfWidth = lattice.edge(lattice.logicalForDrawn(drawn)).getWidth();

        glm::vec2 direction = glm::normalize(B - A);
        glm::vec2 perp = glm::vec2(-direction.y, direction.x) * halfWidth;

        glm::vec2 quad[4] = { A + perp, A - perp, B - perp, B + perp };

        size_t base = drawn * 8;
        for (int c = 0; c < 4; c++) {
            edgeVertices[base + c * 2 + 0] = quad[c].x;
            edgeVertices[base + c * 2 + 1] = quad[c].y;
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, edgeVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * edgeVertices.size(), edgeVertices.data());
}

void FlatRenderer::syncDefects(const Lattice& lattice) {
    const size_t dim = lattice.dimension();
    std::vector<float> data;

    // Plaquettes first, then vertices, so draw() can issue two calls with two
    // colors over one buffer.
    for (size_t row = 0; row < dim; row++) {
        for (size_t col = 0; col < dim; col++) {
            if (lattice.plaquetteFires(col, row)) {
                appendCircle(data, lattice.plaquetteCenter(col, row),
                             RenderConstants::PLAQUETTE_MARKER_RADIUS);
            }
        }
    }
    plaquetteDefectTriangles = data.size() / 6;

    // Grid points run 0..dim inclusive so the seam row/column gets a marker
    // too and the square looks symmetric; the stabilizer itself is looked up
    // modulo dim, since those are the same logical vertex.
    for (size_t row = 0; row <= dim; row++) {
        for (size_t col = 0; col <= dim; col++) {
            if (lattice.vertexFires(col % dim, row % dim)) {
                appendCircle(data, lattice.vertexPosition(col, row),
                             RenderConstants::VERTEX_MARKER_RADIUS);
            }
        }
    }
    vertexDefectTriangles = data.size() / 6 - plaquetteDefectTriangles;

    if (!data.empty()) {
        glBindBuffer(GL_ARRAY_BUFFER, defectVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * data.size(), data.data());
    }
}

void FlatRenderer::draw(const Lattice& lattice, unsigned int shaderProgram) {
    const int colorLocation = glGetUniformLocation(shaderProgram, "u_color");

    // Edges. One draw call per drawn edge because color is a uniform; at ~180
    // edges this is far below the point where batching would be worth the
    // extra complexity of a per-vertex color attribute.
    glBindVertexArray(edgeVAO);
    for (size_t drawn = 0; drawn < lattice.drawnEdgeCount(); drawn++) {
        glm::vec4 color = lattice.edge(lattice.logicalForDrawn(drawn)).getColor();
        glUniform4f(colorLocation, color.r, color.g, color.b, color.a);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT,
                       (const void*)(sizeof(unsigned int) * 6 * drawn));
    }

    glBindVertexArray(dotVAO);
    glUniform4f(colorLocation, 0.5f, 0.5f, 1.0f, 1.0f);
    glDrawArrays(GL_POINTS, 0, (GLsizei)dotCount);

    glBindVertexArray(defectVAO);
    if (plaquetteDefectTriangles > 0) {
        const glm::vec4& c = RenderConstants::PLAQUETTE_DEFECT_COLOR;
        glUniform4f(colorLocation, c.r, c.g, c.b, c.a);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)(plaquetteDefectTriangles * 3));
    }
    if (vertexDefectTriangles > 0) {
        const glm::vec4& c = RenderConstants::VERTEX_DEFECT_COLOR;
        glUniform4f(colorLocation, c.r, c.g, c.b, c.a);
        glDrawArrays(GL_TRIANGLES, (GLint)(plaquetteDefectTriangles * 3),
                     (GLsizei)(vertexDefectTriangles * 3));
    }
}

void FlatRenderer::shutdown() {
    glDeleteBuffers(1, &dotVBO);
    glDeleteBuffers(1, &edgeVBO);
    glDeleteBuffers(1, &edgeEBO);
    glDeleteBuffers(1, &defectVBO);
    glDeleteVertexArrays(1, &dotVAO);
    glDeleteVertexArrays(1, &edgeVAO);
    glDeleteVertexArrays(1, &defectVAO);
}
