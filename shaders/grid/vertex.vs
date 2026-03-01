#version 460 core

uniform mat4 viewProjection;

out vec3 linePos;

const float GRID_EXTENT = 10.0;  // сетка от -10 до +10
const float GRID_STEP   = 1.0;   // шаг линии
const int LINES_COUNT   = 21;

void main() {
    int vertexId = gl_VertexID;         // 0..(LINES_COUNT*4-1)
    int lineIndex = vertexId / 2;       // индекс линии
    int vertexInLine = vertexId % 2;    // 0 или 1 вершина линии

    if (lineIndex < LINES_COUNT) {
        // линии вдоль X (параллельно X, меняется Y)
        float y = -GRID_EXTENT + lineIndex * GRID_STEP;
        float x = vertexInLine == 0 ? -GRID_EXTENT : GRID_EXTENT;
        linePos = vec3(x, y, 0.0); // Z = 0
    } else {
        // линии вдоль Y (параллельно Y, меняется X)
        int idx = lineIndex - LINES_COUNT;
        float x = -GRID_EXTENT + idx * GRID_STEP;
        float y = vertexInLine == 0 ? -GRID_EXTENT : GRID_EXTENT;
        linePos = vec3(x, y, 0.0); // Z = 0
    }

    gl_Position = viewProjection * vec4(linePos, 1.0);
}