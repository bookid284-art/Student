"""Build the NightSchool protagonist and in-place locomotion clips as a GLB."""

import json
import struct
import sys
from collections import defaultdict
from math import cos, copysign, radians, sin, sqrt, tau
from pathlib import Path


FABRICS = {
    'NS_UniformNavy': ((.044, .083, .127), .86),
    'NS_UniformPiping': ((.21, .27, .28), .84),
    'NS_UniformRibbing': ((.025, .046, .069), .9),
    'NS_UniformTrousers': ((.052, .059, .070), .89),
    'NS_UniformShoes': ((.035, .041, .043), .76),
    'NS_UniformSoles': ((.13, .14, .13), .93),
    'NS_UniformBag': ((.095, .12, .098), .9),
    'NS_UniformWebbing': ((.049, .063, .051), .91),
    'NS_UniformPatch': ((.22, .14, .092), .83),
    'NS_UniformSkin': ((.40, .269, .205), .73),
    'NS_UniformHair': ((.025, .022, .022), .65),
    'NS_UniformFrames': ((.012, .017, .021), .5),
    'NS_UniformBook': ((.16, .049, .033), .85),
}

BONES = {
    'root': (None, (0, 0, 0)),
    'pelvis': ('root', (0, 0, 86)),
    'spine': ('pelvis', (0, 0, 106)),
    'head': ('spine', (0, 0, 146)),
}
for side, sign in [('l', -1), ('r', 1)]:
    BONES[f'arm_{side}'] = ('spine', (0, sign * 16, 134))
    BONES[f'forearm_{side}'] = (f'arm_{side}', (0, sign * 21, 111))
    BONES[f'thigh_{side}'] = ('pelvis', (0, sign * 7, 86))
    BONES[f'shin_{side}'] = (f'thigh_{side}', (1, sign * 8, 47))
    BONES[f'foot_{side}'] = (f'shin_{side}', (-1, sign * 8, 9))

GEOMETRY = defaultdict(list)


def normalized(p):
    length = sqrt(sum(value * value for value in p)) or 1
    return tuple(value / length for value in p)


def loft(fabric, rings, joints, sides=18, roundness=1):
    positions, triangles, influences, uvs = [], [], [], []
    for row, ((x, y, z, rx, ry), weights) in enumerate(zip(rings, joints, strict=True)):
        for i in range(sides):
            c, s = cos(tau * i / sides), sin(tau * i / sides)
            positions.append((x + rx * copysign(abs(c)**roundness, c), y + ry * copysign(abs(s)**roundness, s), z))
            influences.append(weights)
            uvs.append((i / sides, row / (len(rings) - 1)))
    for row in range(len(rings) - 1):
        for i in range(sides):
            a, b = row * sides + i, row * sides + (i + 1) % sides
            triangles.extend(((a, b, b+sides), (a, b+sides, a+sides)))
    for row, reverse in ((0, True), (len(rings)-1, False)):
        x, y, z, _, _ = rings[row]
        center = len(positions)
        positions.append((x, y, z))
        influences.append(joints[row])
        uvs.append((.5, float(not reverse)))
        edge = list(range(row * sides, (row + 1) * sides))
        if reverse:
            edge.reverse()
        triangles.extend((center, a, edge[(i + 1) % sides]) for i, a in enumerate(edge))
    normals = [[0.0, 0.0, 0.0] for _ in positions]
    for a, b, c in triangles:
        u = [positions[b][i] - positions[a][i] for i in range(3)]
        v = [positions[c][i] - positions[a][i] for i in range(3)]
        n = (u[1]*v[2]-u[2]*v[1], u[2]*v[0]-u[0]*v[2], u[0]*v[1]-u[1]*v[0])
        for vertex in (a, b, c):
            for i, value in enumerate(n):
                normals[vertex][i] += value
    GEOMETRY[f'NS_Uniform{fabric}'].append((positions, [normalized(v) for v in normals], triangles, influences, uvs))


def rigid(fabric, joint, rings, sides=18, roundness=1):
    loft(fabric, rings, [((joint, 1.),)] * len(rings), sides, roundness)


def build_outfit():
    rigid('Navy', 'spine', [(0,0,85,8,13),(-.3,0,89,10.2,15.6),(-.7,0,96,10.9,14.8),(-.8,0,110,11.2,15),(-.6,0,124,12,17),(-.8,0,134,10.9,17.8),(-.5,0,140,8.5,13.3),(0,0,143,6.1,8)], 24, .78)
    rigid('Ribbing', 'spine', [(0,0,85,8.6,13.2),(-.2,0,87,10,15.1),(-.3,0,90,10.4,15.6)], 24, .78)
    rigid('Ribbing', 'spine', [(0,0,141,6.3,8),(0,0,144,5.6,7),(.5,0,147,5.1,6.2)], 24, .9)
    rigid('Piping', 'spine', [(10.1,0,90,.75,.48),(10.7,0,107,.75,.48),(11.3,0,124,.75,.48),(9.0,0,136,.75,.48),(6.2,0,143,.6,.42)], 8)
    rigid('Patch', 'spine', [(10.1,-7.8,120,.65,1.5),(10.4,-7.6,121,.72,1.75),(10.3,-7.4,124,.65,1.55)], 12, .43)
    rigid('Bag', 'spine', [(-12,0,91,3.1,8),(-13.7,0,94,5.3,10.6),(-14.4,0,99,6.9,11.7),(-14.8,0,114,7.5,12),(-14.0,0,126,6.4,11),(-12,0,132,4.6,8.4),(-10.5,0,135,2.8,5.6)], 24, .55)
    rigid('Webbing', 'spine', [(-19,0,95,2,8),(-20,0,98,3.1,9.1),(-20,0,106,3.25,9.2),(-19,0,111,2.1,8.0)], 20, .52)
    rigid('Piping', 'spine', [(-21,0,108,.9,8.9),(-21,0,109,.9,8.9)], 16, .22)
    rigid('Patch', 'spine', [(-22.2,-4,112,1,2.3),(-22.2,-4,115,1,2.3)], 12, .22)
    rigid('Skin', 'head', [(0,0,144,4.5,5),(0,0,148,4.8,5.3),(.6,0,153,4.4,5)], 20)
    rigid('Skin', 'head', [(3.2,0,148.4,3.8,3.8),(2.3,0,150,6,5.2),(1.8,0,154,7.25,6.9),(1,0,159,8.1,7.4),(.45,0,165,7.7,7.2),(0,0,169,6.1,5.85),(-.6,0,172,3.65,3.6),(-.8,0,174,.5,.55)], 28, .9)
    rigid('Hair', 'head', [(-5.1,0,152,1.7,5),(-5.0,0,155,3.7,6.45),(-3.9,0,160,6.0,7.45),(-1.8,0,166,8.65,7.9),(-.85,0,170,8.25,7.25),(-.65,0,173.5,6.6,5.85),(-1.6,0,175.5,4.5,4.05),(-3,0,176.3,1.1,1.2)], 28, .86)
    for side, sign in [('l', -1), ('r', 1)]:
        arm, forearm = f'arm_{side}', f'forearm_{side}'
        thigh, shin, foot = f'thigh_{side}', f'shin_{side}', f'foot_{side}'
        upper, lower = ((arm, 1.),), ((forearm, 1.),)
        elbow = ((arm, .45), (forearm, .55))
        y = sign
        loft('Navy', [(4,y*20.5,87,3.6,3.4),(3.5,y*21.2,93,4.1,3.8),(2,y*21.3,104,4.55,4.1),(0,y*21.2,111,4.75,4.5),(-.6,y*20.8,118,5.15,4.8),(-.8,y*19.3,128,5.8,5.15),(-.6,y*17.5,135,5.0,4.9),(-.1,y*16.0,139,3.25,3.6)], [lower,lower,lower,elbow,upper,upper,upper,upper], 20)
        rigid('Ribbing', forearm, [(4.5,y*20.3,86.0,3.55,3.4),(4,y*20.6,90,3.9,3.55)], 16)
        rigid('Skin', forearm, [(5.5,y*20.0,78.8,1.7,2.3),(5,y*20.15,81,2.6,2.8),(4.6,y*20.45,85,3.15,2.9),(4.1,y*20.55,88,2.6,2.5)], 16, .8)
        rigid('Piping', arm, [(-.5,y*24.3,121,.75,.72),(-.85,y*24.5,128,.75,.68),(-1,y*22.5,134,.75,.56),(-1,y*19.6,137,.7,.54)], 10)
        rigid('Webbing', 'spine', [(-6,y*12.3,116,1.8,1.5),(-7,y*12.5,127,1.6,1.55),(-4,y*11.2,137,1.7,1.5),(1.7,y*10.6,140,1.8,1.4),(5.4,y*10.5,141,1.4,1.1)], 12, .54)
        rigid('Webbing', 'spine', [(8,y*11,104,1,1.45),(9.6,y*11.3,118,1.2,1.45),(8,y*10.8,133,1.2,1.45),(5.6,y*10.4,140,1.4,1.25)], 12, .54)
        loft('Trousers', [(-1,y*8,7,4.15,4.1),(-.9,y*8,17,5.15,4.6),(-.3,y*8,34,6.3,5.25),(1.3,y*8,47,6.9,5.8),(.8,y*7.7,57,7.2,6.1),(0,y*7.3,69,7.8,6.85),(-.2,y*6.8,86,8.4,7.6),(-.3,y*6.1,91,8.6,7.1)], [((shin,1.),)]*3 + [((shin,.52),(thigh,.48))] + [((thigh,1.),)]*3 + [((thigh,.5),('pelvis',.5))], 20, .87)
        rigid('Shoes', foot, [(2.7,y*8,1,11.5,5.3),(3.1,y*8,4,12.3,5.4),(2.4,y*8,6.8,10.5,5.1),(-.9,y*8,10.2,6.3,4.5)], 20, .57)
        rigid('Soles', foot, [(3,y*8,.35,12,5.36),(3,y*8,1.45,12.5,5.55),(3,y*8,2.7,12.5,5.55)], 20, .56)
        rigid('Skin', 'head', [(0,y*7.05,154.8,1.1,.85),(0,y*7.4,157,1.65,1.25),(0,y*7.45,159.2,1.75,1.32),(-.2,y*7.1,161.6,1.3,.9)], 12)
        rigid('Frames', 'head', [(2.2,y*7.3,159.6,4.65,.38),(2.8,y*7.4,160.25,4.85,.36)], 14, .70)
    rigid('Frames', 'head', [(7.8,0,159.7,.37,6.4),(8.0,0,160.4,.43,6.4)], 20, .78)
    rigid('Book', 'forearm_r', [(8,21.6,77.5,4.6,1.2),(8.4,21.6,88.2,4.65,1.2)], 16, .21)


def gltf_axis(p, factor=1):
    return tuple(p[i] * factor for i in (0, 2, 1))


def quaternion(x_angle, y_angle, z_angle=0):
    rx, ry, rz = [radians(value) / 2 for value in (x_angle, y_angle, z_angle)]
    x = sin(rx)*cos(ry)*cos(rz) - cos(rx)*sin(ry)*sin(rz)
    y = cos(rx)*sin(ry)*cos(rz) + sin(rx)*cos(ry)*sin(rz)
    z = cos(rx)*cos(ry)*sin(rz) - sin(rx)*sin(ry)*cos(rz)
    w = cos(rx)*cos(ry)*cos(rz) + sin(rx)*sin(ry)*sin(rz)
    return -x, -z, -y, w


class Export:
    def __init__(self):
        self.binary = bytearray()
        self.doc = {'asset': {'version': '2.0', 'generator': 'NightSchool character builder'},
                    'scene': 0, 'scenes': [{'nodes': [0, 1]}], 'accessors': [], 'bufferViews': [],
                    'nodes': [{'name': 'SK_XuZhixia', 'mesh': 0, 'skin': 0}], 'materials': [], 'animations': []}

    def data(self, values, kind='SCALAR', code='f', bounds=False):
        if len(self.binary) % 4:
            self.binary.extend(bytes(4 - len(self.binary) % 4))
        offset = len(self.binary)
        size = {'SCALAR': 1, 'VEC2': 2, 'VEC3': 3, 'VEC4': 4, 'MAT4': 16}[kind]
        for value in values:
            self.binary.extend(struct.pack('<' + str(size) + code, *value))
        view = len(self.doc['bufferViews'])
        self.doc['bufferViews'].append({'buffer': 0, 'byteOffset': offset, 'byteLength': len(self.binary)-offset})
        accessor = {'bufferView': view, 'componentType': {'f':5126, 'H':5123}[code], 'count': len(values), 'type': kind}
        if bounds:
            accessor['min'] = list(map(min, zip(*values)))
            accessor['max'] = list(map(max, zip(*values)))
        self.doc['accessors'].append(accessor)
        return len(self.doc['accessors']) - 1

    def mesh(self):
        primitives = []
        bone_index = {bone: i for i, bone in enumerate(BONES)}
        for fabric, parts in GEOMETRY.items():
            positions, normals, indices, joints, weights, uvs = [], [], [], [], [], []
            for points, directions, triangles, skin, uv in parts:
                offset = len(positions)
                positions.extend(gltf_axis(p, .01) for p in points)
                normals.extend(gltf_axis(n) for n in directions)
                uvs.extend(uv)
                indices.extend((offset + i,) for a, b, c in triangles for i in (a, c, b))
                for vertex in skin:
                    js = [bone_index[j] for j, _ in vertex]
                    ws = [w for _, w in vertex]
                    joints.append(tuple(js + [0] * (4-len(js))))
                    weights.append(tuple(ws + [0] * (4-len(ws))))
            attributes = {'POSITION': self.data(positions, 'VEC3', bounds=True), 'NORMAL': self.data(normals, 'VEC3'),
                          'TEXCOORD_0': self.data(uvs, 'VEC2'), 'JOINTS_0': self.data(joints, 'VEC4', 'H'),
                          'WEIGHTS_0': self.data(weights, 'VEC4')}
            material = len(self.doc['materials'])
            color, roughness = FABRICS[fabric]
            self.doc['materials'].append({'name': fabric, 'pbrMetallicRoughness': {'baseColorFactor': [*color, 1], 'metallicFactor': 0, 'roughnessFactor': roughness}})
            primitives.append({'attributes': attributes, 'indices': self.data(indices, code='H'), 'material': material})
        self.doc['meshes'] = [{'name': 'SK_XuZhixia', 'primitives': primitives}]

    def skeleton(self):
        matrices = []
        self.joint_nodes = {name: i+1 for i, name in enumerate(BONES)}
        for name, (parent, abs_position) in BONES.items():
            parent_pos = BONES[parent][1] if parent else (0, 0, 0)
            local = [a-b for a,b in zip(abs_position, parent_pos)]
            children = [self.joint_nodes[child] for child, (bone, _) in BONES.items() if bone == name]
            node = {'name': name, 'translation': gltf_axis(local, .01)}
            if children:
                node['children'] = children
            self.doc['nodes'].append(node)
            x, y, z = gltf_axis(abs_position, -.01)
            matrices.append((1,0,0,0, 0,1,0,0, 0,0,1,0, x,y,z,1))
        self.doc['skins'] = [{'name': 'XuZhixiaRig', 'skeleton': 1, 'joints': list(self.joint_nodes.values()), 'inverseBindMatrices': self.data(matrices, 'MAT4')}]

    def clip(self, name, length, stride, crouch=0):
        frame_count = round(length*30)
        times = [i/30 for i in range(frame_count+1)]
        clock = self.data([(t,) for t in times], bounds=True)
        motion = defaultdict(list)
        for i in range(frame_count+1):
            phase = tau*i/frame_count
            step = sin(phase)
            lift = stride * sin(2*phase)**2
            hip_y = -.5 * stride * cos(phase)
            hip_height = 86 - 20*crouch + .8*lift
            motion['pelvis', 'translation'].append(gltf_axis((0, hip_y, hip_height), .01))
            motion['spine', 'rotation'].append(quaternion(1.3*step*stride, 4.3*stride + 22*crouch, 2.7*step*stride))
            motion['head', 'rotation'].append(quaternion(-.8*step, -2*stride - 15*crouch, -1.3*step*stride))
            for side, sign in [('l', -1), ('r', 1)]:
                swing = sign*step
                motion[f'arm_{side}', 'rotation'].append(quaternion(-3*sign, 14*stride*swing - 4*crouch, 1.5*sign))
                motion[f'forearm_{side}', 'rotation'].append(quaternion(0, -6 - 13*stride*max(0, swing) - 13*crouch))
                motion[f'thigh_{side}', 'rotation'].append(quaternion(0, -26*crouch - 24*stride*swing))
                motion[f'shin_{side}', 'rotation'].append(quaternion(0, 52*crouch + 37*stride*max(0, swing)))
                motion[f'foot_{side}', 'rotation'].append(quaternion(0, -26*crouch + 8*stride*swing))
        animation = {'name': name, 'channels': [], 'samplers': []}
        for (joint, path), values in motion.items():
            index = self.data(values, 'VEC3' if path == 'translation' else 'VEC4')
            animation['channels'].append({'sampler': len(animation['samplers']), 'target': {'node': self.joint_nodes[joint], 'path': path}})
            animation['samplers'].append({'input': clock, 'output': index, 'interpolation': 'LINEAR'})
        self.doc['animations'].append(animation)

    def save(self, destination):
        self.binary.extend(bytes((-len(self.binary)) % 4))
        self.doc['buffers'] = [{'byteLength': len(self.binary)}]
        header = json.dumps(self.doc, separators=(',', ':')).encode()
        header += b' ' * ((-len(header)) % 4)
        length = 28 + len(header) + len(self.binary)
        destination.write_bytes(struct.pack('<4sII', b'glTF', 2, length) +
                                struct.pack('<I4s', len(header), b'JSON') + header +
                                struct.pack('<I4s', len(self.binary), b'BIN\0') + self.binary)


if __name__ == '__main__':
    build_outfit()
    scene = Export()
    scene.mesh()
    scene.skeleton()
    scene.clip('NS_Idle', 2.0, 0)
    scene.clip('NS_Walk', 1.0, 1.0)
    scene.clip('NS_Run', .67, 1.45)
    scene.clip('NS_CrouchIdle', 2.0, 0, 1)
    scene.clip('NS_CrouchWalk', 1.2, .56, 1)
    scene.save(Path(sys.argv[1]) if len(sys.argv) > 1 else Path(__file__).with_name('SK_XuZhixia.glb'))
