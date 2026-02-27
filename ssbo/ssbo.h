#pragma once
#include "../gl/glincludes.h"
#include <vector>
#include <algorithm>

class SSBO {
    public:
        GLuint id = 0;
        size_t elementSize = 0;
        uint32_t capacity = 0;
        uint32_t count = 0;
        GLenum usage = GL_DYNAMIC_DRAW;

        SSBO() = default;

        void Create(size_t elemSize, uint32_t initialCapacity = 256, GLenum usageHint = GL_DYNAMIC_DRAW) {
            elementSize = elemSize;
            capacity = initialCapacity;
            usage = usageHint;
            glGenBuffers(1, &id);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
            glBufferData(GL_SHADER_STORAGE_BUFFER, capacity * elementSize, nullptr, usage);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
            count = 0;
        }

        template<typename T>
        void Upload(const std::vector<T>& data) {
            if (data.empty()) {
                return;
            }
            if (count + data.size() >= capacity) {
                uint32_t newCapacity = std::max(count + static_cast<uint32_t>(data.size()), capacity * 2);
                GLuint newBuffer;
                glGenBuffers(1, &newBuffer);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, newBuffer);
                glBufferData(GL_SHADER_STORAGE_BUFFER, newCapacity * elementSize, nullptr, usage);
                if (count > 0) {
                    glBindBuffer(GL_COPY_READ_BUFFER, id);
                    glBindBuffer(GL_COPY_WRITE_BUFFER, newBuffer);
                    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, count * elementSize);
                    glBindBuffer(GL_COPY_READ_BUFFER, 0);
                    glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
                }
                glDeleteBuffers(1, &id);
                id = newBuffer;
                capacity = newCapacity;
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
            }
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, count * elementSize, data.size() * elementSize, data.data());
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
            count += static_cast<uint32_t>(data.size());
        }

        template<typename T>
        void Update(uint32_t index, const T& data) {
            if (index >= count) {
                return;
            }
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, index * elementSize, elementSize, &data);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        }

        template<typename T>
        void Remove(uint32_t index) {
            if (index >= count) {
                return;
            }
            if (index != count - 1) {
                T lastElement;
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
                glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, (count - 1) * elementSize, elementSize, &lastElement);
                glBufferSubData(GL_SHADER_STORAGE_BUFFER, index * elementSize, elementSize, &lastElement);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
            }
            count--;
        }

        void Clear() {
            count = 0;
        }
};