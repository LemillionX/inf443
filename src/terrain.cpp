
#include "terrain.hpp"
#include "math.h" // for RAND, and rand

using namespace vcl;


// Evaluate 3D position of the terrain for any (u,v) \in [0,1]
vec3 evaluate_terrain(float u, float v)
{
    //Scaling
    float const x = 100*(u-0.5f); 
    float const y = 100*(v-0.5f);
    int const n = 5;
    vec2* const p = new vec2[n]{
        vec2(1.0f, 0.0f),
        vec2(0.5f, 0.5f ),
        vec2(0.2f, 0.7f),
        vec2(0.8f, 0.7f),
        vec2(-0.3f, 0.2f)
    };


    float* const h = new float[n] {
        10.0f, -8.0f, 1.0f, 2.0f, -3.0f
    };

    float* const sigma = new float[n]{
        0.5f, 0.15f, 0.2f, 0.2f, 0.72f
    };


    float* d = new float[n];
    for (int i = 0; i < n; i++) {
        d[i] = norm(vec2(u, v) - p[i]) / sigma[i];
    }

    float z = 0.0f;

    for (int i = 0; i < 4; i++) {
        z += h[i] * std::exp(- d[i] * d[i]);
    }

    return {x,y,z*noise_perlin({ u,v }, 4, 0.7f, 3.0f) };
}

mesh create_terrain()
{
    // Number of samples of the terrain is N x N
    const unsigned int N = 100;

    mesh terrain; // temporary terrain storage (CPU only)
    terrain.position.resize(N*N);
    terrain.uv.resize(N * N);

    // Fill terrain geometry
    for(unsigned int ku=0; ku<N; ++ku)
    {
        for(unsigned int kv=0; kv<N; ++kv)
        {
            // Compute local parametric coordinates (u,v) \in [0,1]
            const float u = ku/(N-1.0f);
            const float v = kv/(N-1.0f);

            // Compute the local surface function
            vec3 const p = evaluate_terrain(u, v) ;
            // Store vertex coordinates
            terrain.position[kv+N*ku] = p;
            terrain.uv[kv + N * ku] = { (N/10)*u,(N/10)*v };
        }
    }

    // Generate triangle organization
    //  Parametric surface with uniform grid sampling: generate 2 triangles for each grid cell
    for(size_t ku=0; ku<N-1; ++ku)
    {
        for(size_t kv=0; kv<N-1; ++kv)
        {
            const unsigned int idx = kv + N*ku; // current vertex offset

            const uint3 triangle_1 = {idx, idx+1+N, idx+1};
            const uint3 triangle_2 = {idx, idx+N, idx+1+N};

            terrain.connectivity.push_back(triangle_1);
            terrain.connectivity.push_back(triangle_2);
        }
    }

	terrain.fill_empty_field(); // need to call this function to fill the other buffer with default values (normal, color, etc)
    return terrain;
}


bool is_valid_position(vec3 p, std::vector<vcl::vec3> position, float dimension) {
    for (vec3 previous : position) {
        float norm = pow((previous[0] - p[0]), 2) + pow((previous[1] - p[1]), 2); //la distance au carr�
        if (norm < 2 * pow(dimension, 2) || !(-50.0f < p[0]) || !(p[0] < 50.0f) || !(-50.0f < p[1]) || !(p[1] < 50.0f)) {
            return false;
        }
    }
    return true;
}


std::vector<vcl::vec3> generate_positions_on_terrain(int N, float dimension) {
    std::vector<vcl::vec3> particules;

    //Chargement d'un premier batiment
    vec3 p_0 = { 0.0f, 0.0f, 0.0f };
    const float u_0 = rand_interval(0, 1);
    const float v_0 = rand_interval(0, 1);
    p_0 = evaluate_terrain(u_0, v_0);
    particules.push_back(p_0);

    //Chargement des batiments alentours
    float r; //la proba d'�tre un batiment isol�
    int i = 1; //le nombre de batiments d�j� implant�s
    vec3 p = { 0.0f, 0.0f, 0.0f };
    while (i < N) {
        r = rand_interval(); //nombre entre 0 et 1

        if (r < 0.2f) {         //alors le batiment est ind�pendant des b�timents existants
            const float u = rand_interval();
            const float v = rand_interval();
            p = evaluate_terrain(u, v);
        }
        else {                  //alors le batiment est aux alentours d'un b�timent pr�c�dent
            const int i_0 = rand() % i; //le batiment autour duquel on construit, entre 0 et i-1
            const float u = rand_interval(0.01f, 0.1f);
            const float v = rand_interval(0.01f, 0.1f);
            p = evaluate_terrain(u + (particules[i_0][0] / 100.0f + 0.5f), v + (particules[i_0][1] / 100.0f + 0.5f));

        }
        if (is_valid_position(p, particules, dimension)) {
            particules.push_back(p);
            i++;
            //std::cout << p << std::endl;
        }
    }
    return particules;
}

