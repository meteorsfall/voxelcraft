#ifndef _WORLD_HPP_
#define _WORLD_HPP_

#include "utils.hpp"
#include "aabb.hpp"
#include "chunk.hpp"
#include "megachunk.hpp"
#include "texture_atlasser.hpp"
#include "universe.hpp"

/// \cond HIDDEN_SYMBOLS

class IVec3Hasher
{
public:
  size_t operator() (ivec3 const& key) const
  {
    return hash_ivec3(key);
  }
};
class IVec3EqualFn
{
public:
  bool operator() (ivec3 const& t1, ivec3 const& t2) const
  {
    return t1 == t2;
  }
};

/// \endcond

/**
 *\addtogroup VoxelEngine
 * @{
 */

/// Callback type for a collision event. When called, it will give a translation vector for how to no longer be colliding, and the coefficient of friction.
using fn_on_collide = std::function<void(vec3, float)>;

/// The World class refers to a single VoxelEngine world

class World {
public:
    /// The world ID that uniquely identifies this world
    int world_id;

    /// Creates a new world with no loaded chunks
    World();

    /// Sets a block to the given blocktype
    void set_block(int x, int y, int z, int model);
    
    /// Retrives the blockdata for viewing purposes.
    const BlockData* read_block(int x, int y, int z);
    /// Retrives the blockdata for viewing purposes.
    const BlockData* read_block(ivec3 location);

    /// Retrieves the blockdata for writing purposes. Will trigger a chunk rerender
    BlockData* write_block(int x, int y, int z);
    /// Retrieves the blockdata for writing purposes. Will trigger a chunk rerender
    BlockData* write_block(ivec3 location);

    /// Mark a chunk for rendering.
    /**
     * When rendering the world, by default no chunks will be rendered. If you want to render a chunk,
     * then you must mark it before calling @ref World::render. You must mark it every single frame, as calling
     * @ref World::render will reset all previous calls to @ref mark_chunk.
     * 
     * A natural way to use this function is to mark all chunks within a given radius of the player.
     * This is what @ref example_game does, for example.
     * 
     * Note that block faces will be culled if its neighbor is another opaque block, and this culling will occur
     * regardless of whether or not neighboring chunks are actually marked for render.
     * 
     * @param chunk_coords the chunk coordinates of the chunk that is to be marked for rendering
     * @param priority The priority of the chunk render. If several chunks have yet to be rendered,
     * only a single chunk mesh will be generated per frame, as rendering can take 5-10ms.
     * Thus, if only a single chunk may be chosen, then the chunk with the lowest priority value will be rendered first.
     * A chunk with priority 0 is guaranteed to be forcibly rendered, regardless of how long it takes.
     */ 
    void mark_chunk(ivec3 chunk_coords, int priority);

    /// Mark a chunk as having been generated by the world generator. This is to keep track of which chunks have been generated
    void mark_generated(ivec3 chunk_coords);

    /// Returns true if the chunk has been generated by the world generator
    bool is_generated(ivec3 chunk_coords);

    /// Renders the world, based on the marked chunks
    void render(mat4& P, mat4& V, TextureAtlasser& atlasser);

    /// Casts a ray onto the first block that the ray intersects. Returns the intersected block, if any
    /**
     * @param position The origin of the raycast
     * @param direction The direction of the raycast
     * @param max_distance The farthest that the raycast will go
     * @param previous_block If true, will return the block _before_ the one that received the raycast
     * 
     * @returns An ivec3 represents the block that the raycast first intersected, or nullopt if no such block was found
     */
    optional<ivec3> raycast(vec3 position, vec3 direction, float max_distance, bool previous_block = false);

    /// Collide the given AABB collision box with all of the blocks in the world.
    /**
     * 
     * @ref AABB's will indeed move during the physics simulation. However, the physics simulation is not aware
     * of collisions. We use the word `collide` to refer to detecting potentially overlapping @ref AABB's,
     * and then push them so that they are no longer intersecting during this time-step.
     * 
     * For each solid block in the world that is intersecting with the given AABB collision box,
     * collide() will call on on_collide with a vector for how to push the AABB
     * out of the solid block that you're colliding with. For example,
     * this is used to keep the player standing on the ground, and to prevent the player from walking through trees.
     * If you are deep in the ground, on_collide will be called over and over again as it progressively pushes you
     * up and out of the ground.
     * 
     * @param collision_box The collision box that the world must try to push out
     * @param on_collide The callback that will be executed for every collision that the collision_box must go through.
     * on_collide will be given a vec3 showing what translation is needed to exit the collision situation.
     */
    void collide(AABB collision_box, fn_on_collide on_collide);

    /// Save the world to the given filepath
    void save(const char* filepath);
    /// Load the world from the given filepath
    bool load(const char* filepath);
    
private:
    string save_filepath;

    // Map from megachunk_coords to megachunks is here
    unordered_map<ivec3, MegaChunk, IVec3Hasher, IVec3EqualFn> megachunks;
    unordered_map<ivec3, string, IVec3Hasher, IVec3EqualFn> disk_megachunks;

    Chunk* get_chunk(int x, int y, int z);
    BlockData* get_block(int x, int y, int z);
    void refresh_block(int x, int y, int z);

    vector<pair<int, ivec3>> marked_chunks;
    ChunkData* get_chunk_data(ivec3 chunk_coords);
    void load_disk_megachunk(ivec3 megachunk_coords);
    void save_megachunk(ivec3 megachunk_coords, bool keep_in_memory = false);
    Chunk* make_chunk(int x, int y, int z);
    int render_iteration = 0;
};

/**@}*/

#endif
