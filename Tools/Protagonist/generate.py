# 程序化生成虚构的高三理科生静态网格，并导出 UE5 可用的 FBX。
# 无头运行：blender --background --python Tools/Protagonist/generate.py
# 几何单位是米。导出时把米换成厘米写进顶点（100 倍），UE 里约 170–180 单位高。没有骨骼。

import os

import bmesh
import bpy


ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
OUT_PATH = os.path.join(ROOT, "Content", "NightSchool", "Characters", "SM_XuZhixia.fbx")

# 纯色部位。没有贴图。数值是 Principled BSDF 的 Base Color。
COLORS = {
    "M_Skin": (0.62, 0.45, 0.36, 1.0),
    "M_Hair": (0.06, 0.05, 0.045, 1.0),
    "M_Eye": (0.08, 0.07, 0.06, 1.0),
    "M_Jacket": (0.12, 0.18, 0.32, 1.0),
    "M_Shirt": (0.82, 0.82, 0.78, 1.0),
    "M_Trousers": (0.10, 0.11, 0.13, 1.0),
    "M_Shoes": (0.05, 0.05, 0.05, 1.0),
    "M_Glasses": (0.02, 0.02, 0.02, 1.0),
    "M_Backpack": (0.16, 0.17, 0.18, 1.0),
    "M_Notebook": (0.55, 0.16, 0.12, 1.0),
}


def clear_scene():
    bpy.ops.wm.read_factory_settings(use_empty=True)


def make_material(name):
    material = bpy.data.materials.new(name)
    material.use_nodes = True
    nodes = material.node_tree.nodes
    principled = nodes.get("Principled BSDF")
    if principled:
        principled.inputs["Base Color"].default_value = COLORS[name]
        principled.inputs["Roughness"].default_value = 0.55
        if "Specular IOR Level" in principled.inputs:
            principled.inputs["Specular IOR Level"].default_value = 0.3
        elif "Specular" in principled.inputs:
            principled.inputs["Specular"].default_value = 0.3
    return material


def new_object(name, bm, material):
    # 三角化，并把法线翻到外侧，避免进 UE 后出现黑面。
    bmesh.ops.recalc_face_normals(bm, faces=list(bm.faces))
    bmesh.ops.triangulate(bm, faces=list(bm.faces))
    mesh = bpy.data.meshes.new(name)
    bm.to_mesh(mesh)
    bm.free()
    mesh.materials.append(material)
    obj = bpy.data.objects.new(name, mesh)
    bpy.context.scene.collection.objects.link(obj)
    obj.location = (0.0, 0.0, 0.0)
    return obj


def add_box(name, center, size, material):
    bm = bmesh.new()
    bmesh.ops.create_cube(bm, size=1.0)
    sx, sy, sz = size
    cx, cy, cz = center
    for vert in bm.verts:
        vert.co.x = vert.co.x * sx + cx
        vert.co.y = vert.co.y * sy + cy
        vert.co.z = vert.co.z * sz + cz
    return new_object(name, bm, material)


def add_sphere(name, center, radius, material, segments=12, rings=8):
    bm = bmesh.new()
    bmesh.ops.create_uvsphere(bm, u_segments=segments, v_segments=rings, radius=radius)
    cx, cy, cz = center
    for vert in bm.verts:
        vert.co.x += cx
        vert.co.y += cy
        vert.co.z += cz
    return new_object(name, bm, material)


def build():
    materials = {name: make_material(name) for name in COLORS}
    # 面朝 -Y。脚底在 Z=0。目标身高约 1.75 米，导出后约 175 UE 单位。
    add_box("Shoes", (0.0, -0.02, 0.03), (0.22, 0.30, 0.06), materials["M_Shoes"])
    add_box("Legs", (0.0, 0.0, 0.46), (0.24, 0.18, 0.80), materials["M_Trousers"])
    add_box("Torso", (0.0, 0.0, 1.08), (0.38, 0.20, 0.52), materials["M_Jacket"])
    add_box("Neck", (0.0, 0.0, 1.37), (0.10, 0.10, 0.10), materials["M_Skin"])
    add_box("Shirt", (0.0, -0.09, 1.28), (0.16, 0.04, 0.10), materials["M_Shirt"])
    add_box("Arm_L", (-0.26, 0.0, 1.08), (0.10, 0.10, 0.46), materials["M_Jacket"])
    add_box("Arm_R", (0.26, 0.0, 1.08), (0.10, 0.10, 0.46), materials["M_Jacket"])
    add_box("Hand_L", (-0.26, -0.02, 0.82), (0.08, 0.08, 0.08), materials["M_Skin"])
    add_box("Hand_R", (0.26, -0.06, 0.84), (0.08, 0.10, 0.08), materials["M_Skin"])
    add_sphere("Head", (0.0, 0.0, 1.52), 0.12, materials["M_Skin"], 16, 10)
    add_sphere("Hair", (0.0, 0.02, 1.58), 0.125, materials["M_Hair"], 16, 10)
    add_sphere("Eye_L", (-0.045, -0.10, 1.54), 0.012, materials["M_Eye"], 8, 6)
    add_sphere("Eye_R", (0.045, -0.10, 1.54), 0.012, materials["M_Eye"], 8, 6)
    add_box("Glasses", (0.0, -0.115, 1.54), (0.13, 0.012, 0.035), materials["M_Glasses"])
    add_box("Backpack", (0.0, 0.16, 1.12), (0.28, 0.12, 0.36), materials["M_Backpack"])
    add_box("Notebook", (0.26, -0.12, 0.86), (0.10, 0.14, 0.02), materials["M_Notebook"])


def measure():
    highest = 0.0
    for obj in bpy.data.objects:
        if obj.type != "MESH":
            continue
        for vert in obj.data.vertices:
            highest = max(highest, vert.co.z)
    print("HEIGHT_M %.3f" % highest)
    print("HEIGHT_UE %.1f" % (highest * 100.0))
    return highest


def export():
    os.makedirs(os.path.dirname(OUT_PATH), exist_ok=True)
    # 模型按米搭建。FBX_SCALE_NONE 把「1 米 = 100 厘米」乘进顶点，再把
    # UnitScaleFactor 写成 1。UE5 打开 Convert Scene 也不会再乘一次。
    # global_scale 必须是 1。若改成 100，会和单位换算叠成 10000，人会有一百多米高。
    bpy.ops.export_scene.fbx(
        filepath=OUT_PATH,
        use_selection=False,
        object_types={"MESH"},
        apply_scale_options="FBX_SCALE_NONE",
        global_scale=1.0,
        apply_unit_scale=True,
        bake_space_transform=True,
        axis_forward="-Y",
        axis_up="Z",
        mesh_smooth_type="FACE",
        use_mesh_modifiers=False,
        add_leaf_bones=False,
        bake_anim=False,
        path_mode="AUTO",
    )
    print("WROTE", OUT_PATH)


def main():
    clear_scene()
    build()
    height = measure()
    if height < 1.65 or height > 1.85:
        raise RuntimeError("身高不在 1.65 到 1.85 米之间：%.3f" % height)
    export()


if __name__ == "__main__":
    main()
