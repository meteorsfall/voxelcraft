use three_d::*;
use three_d::core::Error;

pub struct BlockMesh {
    position_buffer: VertexBuffer,
    normal_buffer: VertexBuffer,
    index_buffer: ElementBuffer,
    shaders: program::Program,
    pub color: Vec3,
    pub texture: Option<texture::Texture2D>,
    pub diffuse_intensity: f32,
    pub specular_intensity: f32,
    pub specular_power: f32
}

impl BlockMesh
{
    pub fn new(gl: &Gl, indices: &[u32], positions: &[f32], normals: &[f32]) -> Result<Self, Error>
    {
        let position_buffer = VertexBuffer::new_with_static_f32(gl, positions)?;
        let normal_buffer = VertexBuffer::new_with_static_f32(gl, normals)?;
        let index_buffer = ElementBuffer::new_with_u32(gl, indices)?;

        let shaders = program::Program::from_source(&gl,
                                                    include_str!("../assets/shaders/block.vert"),
                                                    include_str!("../assets/shaders/block.frag"))?;

        Ok(BlockMesh { index_buffer, position_buffer, normal_buffer, shaders, color: vec3(1.0, 1.0, 1.0), texture: None,
            diffuse_intensity: 0.5, specular_intensity: 0.2, specular_power: 6.0 })
    }

    pub fn update_positions(&mut self, positions: &[f32]) -> Result<(), Error>
    {
        self.position_buffer.fill_with_static_f32(positions);
        Ok(())
    }

    pub fn update_normals(&mut self, normals: &[f32]) -> Result<(), Error>
    {
        self.normal_buffer.fill_with_static_f32(normals);
        Ok(())
    }

    pub fn render(&self, transformation: &Mat4, camera: &camera::Camera)
    {
        self.shaders.add_uniform_float("diffuse_intensity", &self.diffuse_intensity).unwrap();
        self.shaders.add_uniform_float("specular_intensity", &self.specular_intensity).unwrap();
        self.shaders.add_uniform_float("specular_power", &self.specular_power).unwrap();

        if let Some(ref tex) = self.texture
        {
            self.shaders.add_uniform_int("use_texture", &1).unwrap();
            self.shaders.use_texture(tex,"tex").unwrap();
        }
        else {
            self.shaders.add_uniform_int("use_texture", &0).unwrap();
            self.shaders.add_uniform_vec3("color", &self.color).unwrap();
        }

        self.shaders.add_uniform_mat4("modelMatrix", &transformation).unwrap();
        self.shaders.use_uniform_block(camera.matrix_buffer(), "Camera");

        self.shaders.add_uniform_mat4("normalMatrix", &transformation.invert().unwrap().transpose()).unwrap();

        self.shaders.use_attribute_vec3_float(&self.position_buffer, "position").unwrap();
        self.shaders.use_attribute_vec3_float(&self.normal_buffer, "normal").unwrap();

        self.shaders.draw_elements(&self.index_buffer);
    }
}
