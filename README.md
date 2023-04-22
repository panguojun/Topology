# Topo Tree

Convert any graph structure into a tree structure, and traverse the tree according to certain rules to obtain different chain structures.

## Usage

A shape can be formalized as: shape = {topo, coord, space}.

In addition, the graph corresponds to the equation, and the tree corresponds to the solution structure of the equation.

```cpp
topoE te =  {"te",{0}, false };
topoE te1 = { "te1",{0, 1}, false};
topoE te2 = { "te2",{2, 3}, false};
topoE te3 = { "te3", {0}, true };
topoE te4 = { "te4",{1, 2}, false };
topoE te5 = { "te5",{1, 3}, false };

te.children.emplace_back(&te1);
te.children.emplace_back(&te3);
te3.children.emplace_back(&te4);
te3.children.emplace_back(&te5);
te.children.push_back(&te2);

static const vec3 p_set[] = {
    vec3(-1, -1, 1) ,vec3(0.5, 1, 1), 
    vec3(-2, 1.8, 3), vec3(2, 1.8, -3)
};
{
    VECLIST e;
    vec3 p0;
    // Traverse along the left side
    te.walkL([&e, &p0](topoE* to, real t) {
        coord3 c1(p_set[to->e.start]), c2(p_set[to->e.end]);
        c1.rot(-45, vec3::UZ); c2.rot(45, vec3::UX);
        vec3 p = blender::slerp(c1, c2, t).o;

        if (VECLIST_ADD(e, p)) {
            if (p0 != vec3::ZERO)
                ptr(p0, p, 0.01);
            p0 = p;
        }
    });
    PRINT(e.size() << " = " << COMBINE_STEPS(5,3));
}
```

## Structure

### Edge

An edge object consists of a start point and an end point, and a child list that can be traversed.

```cpp
struct tedge
{
    string name;
    struct edge { int start, end; } e;          // Edge object (leaf node)
    bool bmutex = false;                        // Whether the child list is mutually exclusive

    Lineiter space = { 0, 1, 5 };                // Associate different constraints with parameter space
    int cs = 0;

    tedge* parent = 0;                          // Parent
    std::vector<tedge*> children;               // Child list

    void walkL(std::function<void(tedge* to, real t)> cb)
    {
        if (children.empty()) {
            if (cb)
            {
                space.walk([this, &cb](int i, real t) { cb(this, t); });
            }
            return;
        }
        if (bmutex)
        {
            children.front()->walkL(cb);
        } else {
            for (auto c : children)
                c->walkL(cb);
        }
    }
};
```

### Shape

A shape consists of a topology, coordinate, and space.

```cpp
struct tshape
{
    string name;
    topoE* topo = 0;        // Topology
    coord3* coord = 0;      // Coordinate
    Lineiter* space = 0;    // Space

    tshape(const string& name) : name(name) {}
};
```
