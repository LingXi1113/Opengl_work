import math

def generate_terrain(filename, width, depth, resolution):
    vertices = []
    uvs = []
    normals = []
    faces = []

    # Generate vertices
    for z in range(resolution + 1):
        for x in range(resolution + 1):
            x_pos = (x / resolution - 0.5) * width
            z_pos = (z / resolution - 0.5) * depth
            
            # Height function (Simple waves to simulate hills)
            # You can make this more complex with Perlin noise
            y_pos = 3.0 * math.sin(x_pos * 0.2) + 2.0 * math.cos(z_pos * 0.15) + 1.0 * math.sin(z_pos * 0.5)

            vertices.append((x_pos, y_pos, z_pos))
            
            # UVs (Repeat texture 10 times across the terrain)
            uvs.append((x / resolution * 10.0, z / resolution * 10.0))
            
            # Normals (Simplified, pointing up-ish)
            # For perfect lighting, we should calculate cross product of neighbors
            normals.append((0.0, 1.0, 0.0)) 

    # Generate faces
    for z in range(resolution):
        for x in range(resolution):
            # Indices of the 4 corners of the quad
            top_left = z * (resolution + 1) + x
            top_right = top_left + 1
            bottom_left = (z + 1) * (resolution + 1) + x
            bottom_right = bottom_left + 1

            # Two triangles per quad
            # OBJ indices are 1-based
            faces.append((top_left + 1, bottom_left + 1, top_right + 1))
            faces.append((top_right + 1, bottom_left + 1, bottom_right + 1))

    # Write to OBJ file
    with open(filename, 'w') as f:
        f.write("# Procedural Terrain\n")
        f.write("o Terrain\n")
        
        for v in vertices:
            f.write(f"v {v[0]:.4f} {v[1]:.4f} {v[2]:.4f}\n")
            
        for vt in uvs:
            f.write(f"vt {vt[0]:.4f} {vt[1]:.4f}\n")
            
        for vn in normals:
            f.write(f"vn {vn[0]:.4f} {vn[1]:.4f} {vn[2]:.4f}\n")
            
        for face in faces:
            # f v/vt/vn
            f.write(f"f {face[0]}/{face[0]}/{face[0]} {face[1]}/{face[1]}/{face[1]} {face[2]}/{face[2]}/{face[2]}\n")

    print(f"Terrain generated: {filename}")

# Generate a 100x100 terrain with 50x50 grid resolution
generate_terrain('c:/Opengl_work/resources/objects/lawn/terrain.obj', 100, 100, 50)
