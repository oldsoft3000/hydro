#pragma once

#include <memory>

class ObjectBase
{
public:
    ObjectBase();
};

typedef std::shared_ptr<ObjectBase> ObjectBasePtr;
typedef std::shared_ptr<const ObjectBase> ObjectBaseConstPtr;
