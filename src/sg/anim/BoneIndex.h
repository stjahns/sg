#pragma once

class BoneIndex
{
public:

    BoneIndex() : index(Invalid.index)
    {
    }

    BoneIndex(int index) : index(index)
    {
    }

    bool IsValid() const { return index != Invalid.index; }

    operator int() const { return index; }

    bool operator== (const BoneIndex& other) const { return index == other.index; }

    static const BoneIndex Invalid;

private:

    int index;
};