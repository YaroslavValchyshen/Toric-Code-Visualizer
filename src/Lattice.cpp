#include "Lattice.hpp"

Lattice::Lattice(size_t dimension, float cellScale, glm::vec2 origin)
    : dim(dimension), scale(cellScale), gridOrigin(origin) {

    // 2 * N * N logical qubits: one horizontal + one vertical edge per site.
    edgeStates.resize(2 * dim * dim);

    // Drawn edges follow the flat picture, so the border is closed and the
    // grid looks like a normal square lattice. Rows 0..N inclusive means the
    // top row gets drawn even though it IS the bottom row logically.
    const size_t drawnCount = (dim + 1) * dim * 2;
    drawnPoints.reserve(drawnCount * 2);
    drawnToLogical.reserve(drawnCount);

    for (size_t row = 0; row <= dim; row++) {
        for (size_t col = 0; col < dim; col++) {
            drawnPoints.push_back(glm::vec2(gridOrigin.x + col * scale,
                                            gridOrigin.y + row * scale));
            drawnPoints.push_back(glm::vec2(gridOrigin.x + (col + 1) * scale,
                                            gridOrigin.y + row * scale));
            drawnToLogical.push_back(horizontalEdge(row, col)); // row == dim folds to row 0
        }
    }
    for (size_t col = 0; col <= dim; col++) {
        for (size_t row = 0; row < dim; row++) {
            drawnPoints.push_back(glm::vec2(gridOrigin.x + col * scale,
                                            gridOrigin.y + row * scale));
            drawnPoints.push_back(glm::vec2(gridOrigin.x + col * scale,
                                            gridOrigin.y + (row + 1) * scale));
            drawnToLogical.push_back(verticalEdge(col, row)); // col == dim folds to col 0
        }
    }
}

size_t Lattice::horizontalEdge(size_t row, size_t col) const {
    return (row % dim) * dim + (col % dim);
}

size_t Lattice::verticalEdge(size_t col, size_t row) const {
    return dim * dim + (col % dim) * dim + (row % dim);
}

void Lattice::edgeGridSpan(size_t logicalIndex, float& u0, float& v0,
                            float& u1, float& v1) const {
    const size_t horizontalBlock = dim * dim;
    if (logicalIndex < horizontalBlock) {
        size_t row = logicalIndex / dim;
        size_t col = logicalIndex % dim;
        u0 = (float)col;       v0 = (float)row;
        u1 = (float)(col + 1); v1 = (float)row;
    } else {
        size_t local = logicalIndex - horizontalBlock;
        size_t col = local / dim;
        size_t row = local % dim;
        u0 = (float)col; v0 = (float)row;
        u1 = (float)col; v1 = (float)(row + 1);
    }
}

void Lattice::toggle(size_t logicalIndex, bool asXError) {
    if (asXError) edgeStates[logicalIndex].toggleXError();
    else          edgeStates[logicalIndex].toggleHighlight();
}

std::array<size_t, 4> Lattice::plaquetteEdges(size_t col, size_t row) const {
    return {
        horizontalEdge(row, col),       // bottom
        horizontalEdge(row + 1, col),   // top
        verticalEdge(col, row),         // left
        verticalEdge(col + 1, row)      // right
    };
}

std::array<size_t, 4> Lattice::vertexEdges(size_t col, size_t row) const {
    return {
        horizontalEdge(row, col),                 // right
        horizontalEdge(row, col + dim - 1),       // left  (wraps)
        verticalEdge(col, row),                   // up
        verticalEdge(col, row + dim - 1)          // down  (wraps)
    };
}

size_t Lattice::parityCount(const std::array<size_t, 4>& edges, LineState state) const {
    size_t count = 0;
    for (size_t e : edges) {
        if (edgeStates[e].getState() == state) count++;
    }
    return count;
}

bool Lattice::plaquetteFires(size_t col, size_t row) const {
    return (parityCount(plaquetteEdges(col, row), LineState::XError) % 2) != 0;
}

bool Lattice::vertexFires(size_t col, size_t row) const {
    return (parityCount(vertexEdges(col, row), LineState::ZError) % 2) != 0;
}

glm::vec2 Lattice::plaquetteCenter(size_t col, size_t row) const {
    return glm::vec2(gridOrigin.x + (col + 0.5f) * scale,
                     gridOrigin.y + (row + 0.5f) * scale);
}

glm::vec2 Lattice::vertexPosition(size_t col, size_t row) const {
    return glm::vec2(gridOrigin.x + col * scale,
                     gridOrigin.y + row * scale);
}
