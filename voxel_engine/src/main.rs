
use three_d::*;
mod block_mesh;

// A block template
struct Block {
    block_id: i32,
    mesh : block_mesh::BlockMesh,
    is_opaque : bool,
}

// A block as a specific coordinate
struct BlockInstance {
    block_id: i32,
    x: i32,
    y: i32,
    z: i32,
}

struct World {
    // Blocks
    blocks : Vec<Block>,
    // A world is made up of a set of blocks
    chunk : Vec<BlockInstance>,
}

impl World {
    // Make a new voxelcraft world
    pub fn new() -> Self {
        return World {
            blocks: vec![],
            chunk: vec![],
        };
    }

    pub fn set_block(&mut self, x : i32, y : i32, z : i32, block_id : i32) {
        self.chunk.push(BlockInstance {block_id, x, y, z});
    }

    pub fn create_block(&mut self, gl : &Gl, texture_data : &[u8]) -> i32 {
        let box_mesh = tri_mesh::MeshBuilder::new().unconnected_cube().build().unwrap();
        // Makes a new mesh from the box tri_mesh
        let mut box_mesh = block_mesh::BlockMesh::new(&gl, &box_mesh.indices_buffer(), &box_mesh.positions_buffer_f32(), &box_mesh.normals_buffer_f32()).unwrap();
        
        // Sets the texture of the box tri_mesh
        box_mesh.texture = Some(texture::Texture2D::new_from_bytes(&gl, Interpolation::Linear, Interpolation::Linear, Some(Interpolation::Linear),
                           Wrapping::ClampToEdge, Wrapping::ClampToEdge, texture_data).unwrap());
        
        let b = Block {
            block_id: self.blocks.len() as i32,
            mesh: box_mesh,
            is_opaque: true,
        };

        let b_id = b.block_id;

        self.blocks.push(b);

        return b_id;
    }

    pub fn render(&self, width: usize, height: usize, camera : &Camera) {
        /*
        renderer.geometry_pass(width, height, &|| {
            // Draw box
            for block_instance in &self.chunk {
                let transformation = Mat4::new(
                    1.0, 0.0, 0.0, 0.0,
                    0.0, 1.0, 0.0, 0.0,
                    0.0, 0.0, 1.0, 0.0,
                    (2 * block_instance.x) as f32, (2 * block_instance.y) as f32, (2 * block_instance.z) as f32, 1.0
                );
                let block_id = block_instance.block_id;
                for block in &self.blocks {
                    if block.block_id == block_id {
                        block.mesh.render(&transformation, &camera);
                    }
                }
            }
        }).unwrap();
        */
        // Draw box
        for block_instance in &self.chunk {
            let transformation = Mat4::new(
                1.0, 0.0, 0.0, 0.0,
                0.0, 1.0, 0.0, 0.0,
                0.0, 0.0, 1.0, 0.0,
                (2 * block_instance.x) as f32, (2 * block_instance.y) as f32, (2 * block_instance.z) as f32, 1.0
            );
            let block_id = block_instance.block_id;
            for block in &self.blocks {
                if block.block_id == block_id {
                    block.mesh.render(&transformation, &camera);
                }
            }
        }

    }
}

fn main() {
    let mut world = World::new();

    let args: Vec<String> = std::env::args().collect();
    let screenshot_path = if args.len() > 1 { Some(args[1].clone()) } else {None};

    let mut window = Window::new_default("Texture").unwrap();
    let (width, height) = window.framebuffer_size();
    let gl = window.gl();

    // Makes a new camera with the given location and direction
    let mut camera = Camera::new_perspective(&gl, vec3(5.0, -3.0, 5.0), vec3(0.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0),
                                                degrees(45.0), width as f32 / height as f32, 0.1, 1000.0);

    // Make an "unconnected cube" tri_mesh
    let stone = world.create_block(&gl, include_bytes!("../assets/textures/stone.png"));
    let dirt = world.create_block(&gl, include_bytes!("../assets/textures/dirt.jpg"));

    world.set_block(0,0,0,stone);
    world.set_block(1,1,1,dirt);
    world.set_block(2,2,2,stone);
    
    // Makes a Texture Cube
    let texture3d = TextureCubeMap::new_from_bytes(&gl, Interpolation::Linear, Interpolation::Linear, None, Wrapping::ClampToEdge, Wrapping::ClampToEdge, Wrapping::ClampToEdge,
                                                       include_bytes!("../assets/textures/skybox_evening/back.jpg"),
                                                       include_bytes!("../assets/textures/skybox_evening/front.jpg"),
                                                       include_bytes!("../assets/textures/skybox_evening/top.jpg"),
                                                       include_bytes!("../assets/textures/skybox_evening/left.jpg"),
                                                       include_bytes!("../assets/textures/skybox_evening/right.jpg")).unwrap();
    // Makes a skybox from the texture cube
    let skybox = objects::Skybox::new(&gl, texture3d);

    let ambient_light_effect = ImageEffect::new(&gl, include_str!("../assets/shaders/ambient_light.frag")).unwrap();

    // Create ambient light
    let ambient_light = AmbientLight::new(&gl, 0.4, &vec3(1.0, 1.0, 1.0)).unwrap();

    // Create a directional light
    let directional_light = DirectionalLight::new(&gl, 1.0, &vec3(1.0, 1.0, 1.0), &vec3(0.0, -1.0, -1.0)).unwrap();

    // main loop
    let mut rotating = false;
    window.render_loop(move |frame_input|
    {
        camera.set_size(frame_input.screen_width as f32, frame_input.screen_height as f32);

        // Looping through all input events
        for event in frame_input.events.iter() {
            match event {
                Event::MouseClick {state, button, ..} => {
                    // Keep track of whether or not we're rotating
                    rotating = *button == MouseButton::Left && *state == State::Pressed;
                },
                Event::MouseMotion {delta} => {
                    if rotating {
                        // Rotate the camera
                        let target = camera.target();
                        let pos = camera.position();
                        let mut dir = (target - pos).normalize();
                        let up = camera.up();
                        let left = up.cross(dir);
                        dir += (camera.up() * (delta.1 as f32) + left * (delta.0 as f32)) / 100.0;
                        dir.normalize();
                        camera.set_view(*pos, pos + dir, *up);
                        //camera.rotate(delta.0 as f32, delta.1 as f32);
                    }
                },
                Event::MouseWheel {delta} => {
                    // Zoom in a given direction
                    camera.zoom(*delta as f32);
                },
                Event::Key {state, kind} => {
                    println!("{}", *kind);
                    match state {
                        State::Pressed => {
                            let forward = (camera.target() - camera.position()).normalize();
                            let right = forward.cross(*camera.up());
                            match kind.as_str() {
                                "W" => {
                                    //println!("{} {} {}", forward.x, forward.y, forward.z);
                                    camera.translate(&forward);
                                }
                                "S" => {
                                    let backward = -forward;
                                    camera.translate(&backward);
                                }
                                "A" => {
                                    let left = -right;
                                    camera.translate(&left);
                                }
                                "D" => {
                                    camera.translate(&right);
                                }
                                "LShift" => {
                                    camera.translate(&vec3(0.0, 1.0, 0.0));
                                }
                                "LControl" => {
                                    camera.translate(&vec3(0.0, -1.0, 0.0));
                                }
                                _ => {}
                            }
                        }
                        State::Released => {

                        }
                    }
                }
                _ => {}
            }
        }
        
        // Draw all geometry
        let geometry_pass_texture = Texture2DArray::new(&gl, width, height, 2,
            Interpolation::Nearest, Interpolation::Nearest, None, Wrapping::ClampToEdge,
            Wrapping::ClampToEdge, Format::RGBA8).unwrap();
        let geometry_pass_depth_texture = Texture2DArray::new(&gl, width, height, 1,
            Interpolation::Nearest, Interpolation::Nearest, None, Wrapping::ClampToEdge,
            Wrapping::ClampToEdge, Format::Depth32F).unwrap();

        RenderTarget::write_array(&gl,0, 0, width, height,
            Some(&vec4(0.0, 0.0, 0.0, 0.0)), Some(1.0),
            Some(&geometry_pass_texture), Some(&geometry_pass_depth_texture),
            2, &|channel| {channel},
            0, &|| {
            world.render(width, height, &camera);
        });
        
        // Clear frame and set parameters
        gl.viewport(0, 0, width, height);
        gl.bind_framebuffer(consts::DRAW_FRAMEBUFFER, None);
        gl.clear_color(0.8, 0.0, 0.0, 1.0);
        gl.clear(consts::COLOR_BUFFER_BIT);
        
        // Draw the skybox
        skybox.render(&camera).unwrap();

        // Draw light
        state::depth_write(&gl, false);
        state::depth_test(&gl, state::DepthTestType::None);
        state::blend(&gl, state::BlendType::None);
        
        ambient_light_effect.program().use_texture(&geometry_pass_texture, "gbuffer").unwrap();
        ambient_light_effect.program().use_texture(&geometry_pass_depth_texture, "depthMap").unwrap();
        ambient_light_effect.program().add_uniform_vec3("ambientLight.base.color", &ambient_light.color()).unwrap();
        ambient_light_effect.program().add_uniform_float("ambientLight.base.intensity", &ambient_light.intensity()).unwrap();
        ambient_light_effect.apply();
        state::blend(&gl, state::BlendType::OneOne);
        
        state::depth_write(&gl, true);
        state::depth_test(&gl, state::DepthTestType::LessOrEqual);
        state::cull(&gl, state::CullType::None);
        state::blend(&gl, state::BlendType::None);
    }).unwrap();
}
