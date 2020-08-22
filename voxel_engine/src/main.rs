
use three_d::*;

struct Block {
    x: f32,
    y: f32,
    z: f32,
}

struct World {
    blocks : Vec<Block>,
}

impl World {
    pub fn new() -> Self {
        return World {
            blocks: vec![],
        };
    }

    pub fn set_block(&mut self, x : f32, y : f32, z : f32) {
        self.blocks.push(Block {x, y, z});
    }

    fn create_block() {
    
    }
}

fn main() {
    let mut world = World::new();
    world.set_block(0.0, 0.0, 0.0);
    world.set_block(5.0, 0.0, 0.0);

    let args: Vec<String> = std::env::args().collect();
    let screenshot_path = if args.len() > 1 { Some(args[1].clone()) } else {None};

    let mut window = Window::new_default("Texture").unwrap();
    let (width, height) = window.framebuffer_size();
    let gl = window.gl();

    // Makes and Renderer
    let mut renderer = DeferredPipeline::new(&gl).unwrap();

    // Makes a new camera with the given location and direction
    let mut camera = Camera::new_perspective(&gl, vec3(5.0, -3.0, 5.0), vec3(0.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0),
                                                degrees(45.0), width as f32 / height as f32, 0.1, 1000.0);

    // Make an "unconnected cube" tri_mesh
    let box_mesh = tri_mesh::MeshBuilder::new().unconnected_cube().build().unwrap();
    // Makes a new mesh from the box tri_mesh
    let mut box_mesh = Mesh::new(&gl, &box_mesh.indices_buffer(), &box_mesh.positions_buffer_f32(), &box_mesh.normals_buffer_f32()).unwrap();
    
    // Sets the texture of the box tri_mesh
    box_mesh.texture = Some(texture::Texture2D::new_from_bytes(&gl, Interpolation::Linear, Interpolation::Linear, Some(Interpolation::Linear),
                       Wrapping::ClampToEdge, Wrapping::ClampToEdge, include_bytes!("../assets/textures/test_texture.jpg")).unwrap());

    // Makes a Texture Cube
    let texture3d = TextureCubeMap::new_from_bytes(&gl, Interpolation::Linear, Interpolation::Linear, None, Wrapping::ClampToEdge, Wrapping::ClampToEdge, Wrapping::ClampToEdge,
                                                       include_bytes!("../assets/textures/skybox_evening/back.jpg"),
                                                       include_bytes!("../assets/textures/skybox_evening/front.jpg"),
                                                       include_bytes!("../assets/textures/skybox_evening/top.jpg"),
                                                       include_bytes!("../assets/textures/skybox_evening/left.jpg"),
                                                       include_bytes!("../assets/textures/skybox_evening/right.jpg")).unwrap();
    // Makes a skybox from the texture cube
    let skybox = objects::Skybox::new(&gl, texture3d);

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
                        camera.rotate(delta.0 as f32, delta.1 as f32);
                    }
                },
                Event::MouseWheel {delta} => {
                    // Zoom in a given direction
                    camera.zoom(*delta as f32);
                },
                _ => {}
            }
        }

        // Draw all geometry
        renderer.geometry_pass(width, height, &|| {
            // Draw box
            for block in &world.blocks {
                let transformation = Mat4::new(
                    1.0, 0.0, 0.0, 0.0,
                    0.0, 1.0, 0.0, 0.0,
                    0.0, 0.0, 1.0, 0.0,
                    block.x, 0.0, 0.0, 1.0
                );
                box_mesh.render(&transformation, &camera);
            }
        }).unwrap();

        Screen::write(&gl, 0, 0, width, height, Some(&vec4(0.8, 0.0, 0.0, 1.0)), None, &|| {
            // Draw the skybox
            skybox.render(&camera).unwrap();
            // Draw light
            renderer.light_pass(&camera, Some(&ambient_light), &[&directional_light], &[], &[]).unwrap();
        }).unwrap();

        // Not sure what this does?
        if let Some(ref path) = screenshot_path {
            #[cfg(target_arch = "x86_64")]
            Screen::save_color(path, &gl, 0, 0, width, height).unwrap();
            std::process::exit(1);
        }
    }).unwrap();
}
