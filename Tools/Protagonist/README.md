# 许知夏静态网格

虚构的 17–18 岁高三理科生。短发，黑框眼镜，校服外套，双肩书包，偏瘦，手里一本错题本。没有真实人物信息，也没有骨骼。

游戏游玩时仍是仓库里原来的第一人称。这具模型站在光里，供主角第一次看清自己。编辑器没有导入 FBX 之前，同一位置用引擎方块按同样的颜色和部位搭了一个替身。

## 运行

需要本机或云端的 Blender 4.x。在仓库根目录：

```bash
blender --background --python Tools/Protagonist/generate.py
```

脚本会写出 `Content/NightSchool/Characters/SM_XuZhixia.fbx`。几何按米制作。导出使用 `FBX_SCALE_NONE`：Blender 把 1 米写成 100 厘米，顶点大约 170–180，文件里的 UnitScaleFactor 是 1。轴向 Z 朝上，脚底在原点，面朝导出前的 -Y。`global_scale` 保持 1。若再写成 100，单位换算会叠成一万倍，导入后身高会到一万七左右。

## 颜色

没有贴图。每个材质是 Principled BSDF 纯色。

| 材质 | 部位 | Base Color |
| --- | --- | --- |
| M_Skin | 头、脖子、手 | 0.62, 0.45, 0.36 |
| M_Hair | 短发 | 0.06, 0.05, 0.045 |
| M_Eye | 眼睛 | 0.08, 0.07, 0.06 |
| M_Jacket | 校服外套、袖子 | 0.12, 0.18, 0.32 |
| M_Shirt | 领口 | 0.82, 0.82, 0.78 |
| M_Trousers | 裤子 | 0.10, 0.11, 0.13 |
| M_Shoes | 鞋 | 0.05, 0.05, 0.05 |
| M_Glasses | 镜框 | 0.02, 0.02, 0.02 |
| M_Backpack | 书包 | 0.16, 0.17, 0.18 |
| M_Notebook | 错题本 | 0.55, 0.16, 0.12 |

部件名：Shoes、Legs、Torso、Neck、Shirt、Arm_L、Arm_R、Hand_L、Hand_R、Head、Hair、Eye_L、Eye_R、Glasses、Backpack、Notebook。

## 导入 UE5

1. 打开 `Student.uproject`。
2. 把 `SM_XuZhixia.fbx` 拖进 Content Browser 的 `Content/NightSchool/Characters/`。
3. 导入选项：取消 Skeletal Mesh 和 Import Animations。这是静态网格。
4. 缩放保持 1。顶点已经是厘米。Convert Scene 可以开着，不要在 Uniform Scale 里再填 100。
5. 若要拆开部件，取消 Combine Meshes。若只要一个网格，勾上 Combine Meshes。
6. 导入后角色应站立，高度大约 170–180。若躺倒，把导入的 Transform 改回 Z 朝上，不要在脚本外再绕 X 轴 90 度。
7. 法线若出现黑面，在静态网格上做一次 Recompute Normals。脚本导出前已经把面翻到外侧并三角化。

当前脚本不包含骨骼。若以后要让这具身体走路，在 Blender 里给同一网格加 Armature，再单独导出带骨骼的 FBX。第一人称游玩不依赖这具骨骼。
