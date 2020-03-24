#pragma once

#include "../../shader.hpp"
#include "../../computable.hpp"

class MarchingCubesCompute : public Computable<MarchingCubesCompute, glm::vec4> {
public:
    explicit MarchingCubesCompute(ShaderStorageBuffer<glm::vec4>&& ssbo, size_t num_components) 
        : Computable(std::move(ssbo)),
          _shader_stage1(Shader::create(ShaderInfo { "shaders/marching_cubes_stage1.compute", ShaderType::COMPUTE })),
          _shader_stage2(Shader::create(ShaderInfo { "shaders/marching_cubes_stage2.compute", ShaderType::COMPUTE })),
          _num_components(num_components)
    {
    }

    static std::shared_ptr<MarchingCubesCompute> create_impl(int num_components) {
        auto ssbo = ShaderStorageBuffer<glm::vec4>();
        ssbo.reserve_storage(num_components * sizeof(glm::vec4), StorageType::STATIC);
        
        // Reset buffer to zero
        glm::vec4* buffer = ssbo.map_buffer(BufferIntent::WRITE);
        assert(buffer != nullptr);
        for(auto i = 0; i < num_components; i++)
        {
            buffer[i].x = 0.0;
            buffer[i].y = 0.0;
            buffer[i].z = 0.0;
        }
        ssbo.unmap_buffer();

        return std::make_shared<MarchingCubesCompute>(std::move(ssbo), num_components);
    }

    void dispatch_impl(float sea_level) const {
        _shader_stage1.use();
        _shader_stage1.set_float("sea_level", sea_level);
        GL_CHECK(glDispatchCompute(10, 10, 10));
        GL_CHECK(glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT));
    }

    glm::vec4 * expose_buffer() const {
        return _ssbo.map_buffer(BufferIntent::READ);
    }

    void close_buffer() const {
        _ssbo.unmap_buffer();
    }

private:
    Shader _shader_stage1;
    Shader _shader_stage2;
    size_t _num_components;
};