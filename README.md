[![](../../actions/workflows/cpp_cmake.yml/badge.svg)](../../actions)

# Reasonable2D

An OpenGL game engine. Project created at course called "Programowanie Aplikacji Graficznych" (Programming graphic applications) at Lodz University of Technology.

![Screen shot from the game](https://github.com/user-attachments/assets/f9a2cce9-702b-4df9-bcb3-7939ebf4a5bc)

## The project has been successfully completed by implementing following requirements:
- **Physically Based Rendering (PBR):** Fully integrated.
- **Environment Mapping:** Reflective and refractive materials have been applied within the environment.
- **Image-Based Lighting (IBL):** Diffuse and specular components are implemented using IBL techniques.
- **Lighting Support:** Directional, point, and spotlight sources have been added, including gizmo representations. All light properties, such as position, direction, intensity, radius, and color, are adjustable.
- **Shadow Mapping:** Implemented with PCF (Percentage-Closer Filtering) for shadow filtering.
- **Scene Graph:** Procedurally animated objects are added to demonstrate the scene graph functionality. All objects are organized within the scene graph hierarchy.
- **Instanced Rendering:** Successfully implemented for at least 1 million objects in the scene (e.g., grass, trees, etc.).
- **Postprocessing Effects:** Effects such as Bloom have been implemented.
- **Particle Effect with Compute Shader:** 
  - A particle system using Compute Shader has been implemented.  
  - It uses collision-based logic inspired by ![Coding Adventure: Simulating Fluids by Sebastian Lague](https://www.youtube.com/watch?v=rSKMYc1CQHE).  
  - The process of finding asteroids in the lookup table in the collision shader is incorrect. The correct version was lost due to uncommitted changes in GitHub.  
  - (I attempted to add handling for multiple collisions simultaneously but did not upload the working code before making the changes.)
