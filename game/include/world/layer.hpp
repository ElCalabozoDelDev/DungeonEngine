#ifndef LAYER_HPP
#define LAYER_HPP

class Layer
{
public:
    virtual void render() {}
    virtual void update() {}
    protected:
    virtual ~Layer() {}
};

#endif