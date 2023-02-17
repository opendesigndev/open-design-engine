
#pragma once

#include <ode-renderer.h>

using namespace ode;

class RGIShader {
public:
    class SharedVertexShader : public VertexShader {
    public:
        explicit SharedVertexShader(const char *vsLabel);
        bool initialize();
    };

    RGIShader();
    RGIShader(const RGIShader&) = delete;
    RGIShader& operator=(const RGIShader&) = delete;

protected:
    bool initialize(const SharedVertexShader &sharedVertexShader, const char *fsLabel);
    void bind(const ScaledBounds& srcBounds, const ScaledBounds& dstBounds);

    virtual bool initializeFragmentShader(FragmentShader &fragmentShader) const = 0;
    virtual void initializeUniforms() = 0;

    ShaderProgram shader_;
    Uniform unifFraming_;
};
