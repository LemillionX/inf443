#include "tree.hpp"

using namespace vcl;


mesh create_cylinder(float radius, float height)
{
    mesh m;
    // To do: fill this mesh ...
    // ...
    const size_t  n = 30;
    for (size_t i = 0; i < n; ++i) {
        const float u = i / float(n);
        const vec3 p = { radius * std::cos(2.0f * 3.14f * u),  radius * std::sin(2.0f * 3.14f * u), 0.0f };
        m.position.push_back(p);
        m.position.push_back(p + vec3(0.0f, 0.0f, height));
    }

    for (size_t k = 0; k < n; ++k)
    {
        const unsigned int u00 = 2 * k;
        const unsigned int u01 = (2 * k + 1) % (2 * n);
        const unsigned int u10 = (2 * (k + 1)) % (2 * n);
        const unsigned int u11 = (2 * (k + 1) + 1) % (2 * n);

        const uint3 t1 = { u00, u10, u11 };
        const uint3 t2 = { u00, u11, u01 };
        m.connectivity.push_back(t1);
        m.connectivity.push_back(t2);
    }

    m.fill_empty_field();
    return m;
}

mesh create_tree_trunk_cylinder(float radius, float height)
{
    mesh m; 
    // To do: fill this mesh ...
    // ...
    const size_t  n = 30;
    for (size_t i = 0; i < n; ++i) {
        const float u = i / float(n);
        const vec3 p = { radius * std::cos(2.0f * 3.14f * u),  radius * std::sin(2.0f * 3.14f * u), 0.0f };
        m.position.push_back(p);
        m.position.push_back(p+vec3(0.0f, 0.0f, height));
    }

    for (size_t k = 0; k < n; ++k)
    {
        const unsigned int u00 = 2 * k;
        const unsigned int u01 = (2 * k + 1) % (2 * n);
        const unsigned int u10 = (2 * (k + 1)) % (2 * n);
        const unsigned int u11 = (2 * (k + 1) + 1) % (2 * n);

        const uint3 t1 = { u00, u10, u11 };
        const uint3 t2 = { u00, u11, u01 };
        m.connectivity.push_back(t1);
        m.connectivity.push_back(t2);
    }

    m.fill_empty_field();
    return m;
}

mesh create_cone(float radius, float height, float z_offset)
{
    mesh m; 
    // To do: fill this mesh ...
    // ...
    const size_t n = 10;
    for (size_t k = 0; k < n; ++k) {
        const float u = k / float(n);
        const vec3 p = { radius * std::cos(2.0f * 3.14f * u), radius * std::sin(2.0f * 3.14f * u), z_offset };
        m.position.push_back(p);

    }
    m.position.push_back(vec3{ 0,0,z_offset });
    m.position.push_back(vec3{ 0,0,height + z_offset });

    for (size_t k = 0; k < n; k++) {
        const unsigned int u0 = k%(n+2);
        const unsigned int u1 = (k +1) % (n + 2);
        const unsigned int u2 = n;
        const unsigned int u3 = n + 1;

        const uint3 t1 = { u0, u1, u2 };
        const uint3 t2 = { u0, u1, u3 };
        m.connectivity.push_back(t1);
        m.connectivity.push_back(t2);
    }
    const uint3 t1 = { n - 1, 0, n };
    const uint3 t2 = { n - 1, 0, n+1 };
    m.connectivity.push_back(t1);
    m.connectivity.push_back(t2);




    m.fill_empty_field();
    return m;
}


mesh create_tree()
{
    float const h = 0.7f; // trunk height
    float const r = 0.1f; // trunk radius

    // Create a brown trunk
    mesh trunk = create_tree_trunk_cylinder(r, h);
    trunk.color.fill({ 0.4f, 0.3f, 0.3f });


    // Create a green foliage from 3 cones
    mesh foliage = create_cone(4 * r, 6 * r, 0.0f);      // base-cone
    foliage.push_back(create_cone(4 * r, 6 * r, 2 * r));   // middle-cone
    foliage.push_back(create_cone(4 * r, 6 * r, 4 * r));   // top-cone
    foliage.position += vec3(0, 0, h);                 // place foliage at the top of the trunk
    foliage.color.fill({ 0.4f, 0.6f, 0.3f });

    // The tree is composted of the trunk and the foliage
    mesh tree = trunk;
    tree.push_back(foliage);

    return tree;
}

