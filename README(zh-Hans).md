# SmartKnob

[English version](README.md)

SmartKnob 是一款可软件配置限位及虚拟齿槽强度的开源输入设备。

利用无刷云台电机及与其配对的磁性编码器，SmartKnob 可以提供反馈闭环扭矩控制，使其
能够动态创建、调整限位位置和旋转时的齿槽强度（刻度感）。

加入 [Discord 社群](https://discord.gg/5jyhjcbTnR)，与大家一起讨论，展示自己的成品，或者回答他人的问题！

[![Build Status](https://github.com/scottbez1/smartknob/actions/workflows/electronics.yml/badge.svg?branch=master)](https://github.com/scottbez1/smartknob/actions/workflows/electronics.yml)
[![Build Status](https://github.com/scottbez1/smartknob/actions/workflows/pio.yml/badge.svg?branch=master)](https://github.com/scottbez1/smartknob/actions/workflows/pio.yml)

# 设计

## SmartKnob 总览
“SmartKnob 总览”这部分在[我的演示视频](https://www.youtube.com/watch?v=ip641WmY4pA)中集成了显示屏，体验更好。仍在积极开发中。

🎉**电机[有货了](https://www.sparkfun.com/products/20441)！**如果你之前一直在跟进这个项目，
那你肯定知道，我之前推荐的电机在项目发出之后立马就卖断货了。
多亏了[社区里的大家](https://github.com/scottbez1/smartknob/issues/16#issuecomment-1094482805%5D)，我们才能
找到一开始的电机制造商，最近 SparkFun Electronics 也使那种电机复产，并定期
[补货](https://www.sparkfun.com/products/20441)！（不过每次补货之没多久就卖光了，所以如果你想买却缺货，记得开启到货通知）。感谢所有一直在帮助寻找、测试替代
电机的人！

特性：
 - 240 x 240 的圆形 LCD（型号为 GC9A01），在旋转件上用 39.5 毫米直径的手表玻璃做保护
 - 无刷直流云台电机，有可以为 LCD 提供机械支持和电路链接的空心轴
 -  由 ESP32-PICO-V3-02（Lilygo TMicro32 Plus 模组）供电
 - 将 PCB 的形变与贴片电阻结合，用作压力检测机构（通过电机来提供触觉反馈）
 - 利用 8 个侧光 RGB LED 来照亮旋钮周围（型号为 SK6812-SIDE-A）
 - 使用 USB-C (2.0) 接口来提供 5V 供电，实现串口编程和数据传输（利用 CH340 芯片）
 - 利用 VEML7700 环境光传感器来实现自动背光和 LED 亮度调节
 - 用来安装的通用背板：使用 4 枚螺钉，或 2 枚 3M 中号无痕背胶（背板上留有可以在安装完成后再撕下离型纸的缺口）
 - 上盖使用倒扣设计，方便接触 PCB

**现时状态：**不建议大家都来用，但是对有经验的电子爱好者来说可能有点意思。

### 演示视频

<a href="https://www.youtube.com/watch?v=ip641WmY4pA">
    <img src="https://img.youtube.com/vi/ip641WmY4pA/maxresdefault.jpg" width="480" />
</a>

### 工作原理
<a href="https://www.youtube.com/watch?v=Q76dMggUH1M">
    <img src="https://img.youtube.com/vi/Q76dMggUH1M/maxresdefault.jpg" width="480" />
</a>

### 3D CAD

![爆炸图](doc/img/explodedv145.gif)

最新的 Fusion360 模型：https://a360.co/3BzkU0n

### 自己搞一个？

虽然这是个开源的“DIY”项目，但还没有成熟到可以即装即用。如果你打算自己搞一个，那得保证你拥有丰富的焊接经验——有不少距离较近的小型贴片器件焊接（建议直接上回流焊或者热风枪），组装起来也十分耗时、精细。如果你有遇到无数硬件问题、固件问题的心理准备，那就开始动手吧——不过我还是建议你在真正开始之前好好看看、理解一下原理图和基本的固件！

关于 BOM（物料清单）和其他需要订购的部件都会慢慢完善，感谢你的关注！关注我的[推特](https://twitter.com/scottbez1)，及时接收此项目和其他项目的最新消息。

若需要电子元件和硬件的物料清单，可以参见以下自动生成的（也是未经测试的）最新版本[基础 PCB 的交互式 BOM](https://smartknob-artifacts.s3.us-west-1.amazonaws.com/master/electronics/view_base-ibom.html) 和[屏幕 PCB 的交互式 BOM](https://smartknob-artifacts.s3.us-west-1.amazonaws.com/master/electronics/view_screen-ibom.html)（或者你也可以直接查看 [CSV 格式的总和 BOM](https://smartknob-artifacts.s3.us-west-1.amazonaws.com/master/electronics/view_base-bom.csv)）⚠️ 注意，这些清单是基于 Github 上未经测试的修订而自动生成的。而如果需要经过测试的稳定版本，请参看[ Release](https://github.com/scottbez1/smartknob/releases)。

几条小提示：

 - LCD 安装座内部的空间极其有限，而中间的孔里需要穿过 8 条导线。我使用了美规 30 号绕线专用线。漆包线应该也没问题。
 - 视频中的旧版设计使用胶水粘贴的 BF350-3AA 箔式应变片来检测压力；而在 v0.5 中，其被贴片电阻取代。这些电阻在伸缩时会表现出与应变片相似的特性。
 - TMC6300 的元件尺寸*十分微小*，且焊盘在底部，我觉得还是顺道下单个钢网比较好。就算用了钢网，我之后也得手动清理不少锡桥；我*强烈*推荐 Chip Quik NC191 型免洗助焊膏，可在[亚马逊](https://amzn.to/3MGDSr5)（这是[不含推广的链接](https://www.amazon.com/Smooth-Flow-No-Clean-syringe-plunger/dp/B08KJPG3NZ)）或他处购得。在把 LCD 的排线焊到 PCB 上的时候，助焊膏也非常有用。
 - 而如果你只想使用小电流的无刷直流电机搭配面包板，[TMC6300-BOB](https://www.trinamic.com/support/eval-kits/details/tmc6300-bob/) 或者 SparkFun 上的 [TMC6300 driver board](https://www.sparkfun.com/products/21867) 都是不错的选择，比单独的芯片好用不少
 - 如果在美国使用 AliExpress（速卖通）购买，我**只**推荐使用全球速卖通标准运输。我用过几次菜鸟和别的便宜快递，最后苦等几个月却根本没盼来自己的元件；相比起来速卖通标准运输就可靠多了，而且就我的经验来说，也快得多。
 -  记得去看看[尚开放的 ISSUE](https://github.com/scottbez1/smartknob/issues)，因为此设计还远未“稳定”，万事都有可能出问题。

未来规划：
 - 考虑换用 ESP32-S3-MINI-1 模块
 - 配置好 WiFi 功能（可能会使用 MQTT？）。目前的内存是实现完整帧缓冲的瓶颈。PSRAM 大概能解决这个问题（那就需要 ESP-IDF 和尚未发布的 Arduino 内核，我之前已经在一次启用 PSRAM 的小测试中见识到了其可怕的性能），或者用以下方法来降低内存占用：
 - 迁移到 LVGL，以获得更好的显示渲染和更简易的支持菜单等等。这样应该就不需要在内存中划分出完整的 240 x 240 x 24b 帧缓冲区，可以给 WiFi 之类留出一些存储。
 - 与 Home Assistant 或其他的物联网程序集成在一起
 - ???
 - [盈利](https://github.com/sponsors/scottbez1/)😉

#### 已经做好一个了吗？
[软硬件文档](https://paper.dropbox.com/doc/SmartKnob-firmware-and-software--B_oWj~L1dXqHgAqqYmhwwuqzAg-VUb9nq7btuhnHw5KlgJIH)
里有关于将其编程或者链接到自己软件的提示。

如果你已经上传了标准固件，也完成了校准（不知道这是什么的话情参阅前文链接），
那就可以去 https://scottbez1.github.io/smartknob/ 试试基于网页的交互式演示，此演示使用 Web Serial API 来与
插入电脑的 SmartKnob 通信！


#### PCB 基板

<a href="https://smartknob-artifacts.s3.us-west-1.amazonaws.com/master/electronics/view_base-front-3d.png">
    <img src="https://smartknob-artifacts.s3.us-west-1.amazonaws.com/master/electronics/view_base-front-3d.png" width="300" />
</a>
<a href="https://smartknob-artifacts.s3.us-west-1.amazonaws.com/master/electronics/view_base-back-3d.png">
    <img src="https://smartknob-artifacts.s3.us-west-1.amazonaws.com/master/electronics/view_base-back-3d.png" width="300" />
</a>

下单前注意：选择白色阻焊层，以更好地将 RGB LED 的光反射到周围。厚度应为 1.2 毫米，而非“标准”的 1.6 毫米。

如果你打算从嘉立创下单钢网，然后不用植锡台和其他机器，进行纯手工植锡（如
[视频所示](https://youtu.be/Q76dMggUH1M?t=372)），记得选择
**“定制尺寸”**，然后输入个小点的数值（比如 100 毫米 x 100 毫米），防止最后送来的板子
太大。这也可能大大降低运输成本！此外，只选择**正面**贴片，因为底面
只有两个贴片件：电机接口和 VEML7700 ALS，为它俩单开个钢网不值当的。

自动生成的最新文件（未测试过，可能不起作用！）⚠️：

[原理图](https://smartknob-artifacts.s3.us-west-1.amazonaws.com/master/electronics/view_base-schematic.pdf)

[交互式 BOM](https://smartknob-artifacts.s3.us-west-1.amazonaws.com/master/electronics/view_base-ibom.html)

[PCB 包](https://smartknob-artifacts.s3.us-west-1.amazonaws.com/master/electronics/view_base-pcb-packet.pdf)

[Gerbers 包](https://smartknob-artifacts.s3.us-west-1.amazonaws.com/master/electronics/view_base-jlc/gerbers.zip)

⚠️如果需要经过测试的稳定版本，请参看[ Release](https://github.com/scottbez1/smartknob/releases)。

#### 屏幕 PCB

<a href="https://smartknob-artifacts.s3.us-west-1.amazonaws.com/master/electronics/view_screen-front-3d.png">
    <img src="https://smartknob-artifacts.s3.us-west-1.amazonaws.com/master/electronics/view_screen-front-3d.png" width="300" />
</a>
<a href="https://smartknob-artifacts.s3.us-west-1.amazonaws.com/master/electronics/view_screen-back-3d.png">
    <img src="https://smartknob-artifacts.s3.us-west-1.amazonaws.com/master/electronics/view_screen-back-3d.png" width="300" />
</a>

下单前注意：都必须是 1.2 毫米厚，而非“标准”的 1.6 毫米。（PCB 丝印上提到了 0.6 毫米，这说的是用来粘贴 LCD 的 **VHB 胶带**的厚度，PCB 的厚度应为 1.2 毫米）

屏幕 PCB 上的元件不多，我选择使用手工焊接而非回流焊
或者新开钢网，但如果你打算使用钢网，记得参考上面提到的“自定义尺寸”部分，来降低
后续处理难度和运输费用。此外，还要选择仅**底面**，因为所有的元件都在屏幕 PCB 的
底面。

自动生成的最新文件（未测试过，可能不起作用！）⚠️：

[原理图](https://smartknob-artifacts.s3.us-west-1.amazonaws.com/master/electronics/view_screen-schematic.pdf)

[交互式 BOM](https://smartknob-artifacts.s3.us-west-1.amazonaws.com/master/electronics/view_screen-ibom.html)

[PCB 包](https://smartknob-artifacts.s3.us-west-1.amazonaws.com/master/electronics/view_screen-pcb-packet.pdf)

[Gerbers 包](https://smartknob-artifacts.s3.us-west-1.amazonaws.com/master/electronics/view_screen-jlc/gerbers.zip)

⚠️如果需要经过测试的稳定版本，请参看[ Release](https://github.com/scottbez1/smartknob/releases)。

#### 打印件
需要六个打印件。目前的稳定版设计可以在 [v185 mechanical release](https://github.com/scottbez1/smartknob/releases/tag/releases%2Fmechanical%2Fv185-dummy-tag) 中的STL 组件中找到；你也可以在 Fusion 360 中打开以下 CAD 模型：https://a360.co/3BzkU0n ，然后导出未经测试的设计。

这些打印件*应该*能用调整好的 FDM 3D打印机打印出来，但是为了控制公差、表面美观，视频和图片中的打印件都是用 MJF 尼龙打印的。

如果你想再简单点，完全可以省略掉 LCD，直接把模型里的旋钮外壳和玻璃合并成单独的 STL 组件，这样就有个无开窗的旋钮了。

导出并打印以下八个打印件：
* `Enclosure`
* `Knob` (RotorAssembly->KnobAssembly->Knob)
* `ScreenPlatform`
* `RotorSpacer` (RotorAssembly->RotorSpacer)
* `MountBase`
* `BackPlate`

⚠️ 如果你打算在嘉立创下单 MJF 尼龙打印件，他们的系统可能会在打印 `MountBase`、`ScreenPlatform` 和 `RotorSpacer` 的时候弹警告说`“检测到壁厚小于 0.8 毫米”`。貌似是有些喷嘴会出现的误报。这条警告应该可以忽略，因为嘉立创的所有打印单最后都要经过人工审核，如果有任何严重到无法继续的问题，他们会通过邮件跟你联系的。

## NanoFOC（第三方）
要是你想玩玩 FOC 控制或者触觉反馈，却又不想大张旗鼓做一整个 SmartKnob 出来的话，我
推荐你去试试 NanoFOC DevKit++，一款由 SmartKnob 社区成员制造并
[销售](https://store.binaris.io/products/nanofoc-devkit)的[开源设计](https://github.com/katbinaris/nanofoc_devkit)！它十分简洁
，不管是将其用作测试平台还是核心模块，都能帮你打造基于无刷电机的触感输入设备。

![Image of the NanoFOC PCB](https://cdn.shopify.com/s/files/1/0729/7433/6335/products/IMG_20230418_120554-01.jpg?width=416)


NanoFOC 使用 ESP32-S3，SmartKnob 固件在其上开箱即用，只需记得在上传时选择 `nanofoc` 
环境而非 `view`。

# 常见问题(FAQ)

### 我在淘宝上找到的这个便宜无刷电机能用吗？

我拦不住你，但是按照预期的应用方向来看，**你应该也不会有多满意**。

基本上，社区里的大家测试过的所有无刷云台电机（速卖通上的便宜货基本都测了，相信我）都
*或多或少会有些强到过分的齿槽效应（棘轮感）*。也就是说，即使是在断电（软件没有设置阻力）的情况下，电机也会在某些位置有卡顿。如此一来，电机就没法
十分平稳、顺滑地旋转。电机自身的齿槽效应可能会干扰软件的设置，甚至使其完全不可用，这种情况在
试图将软件内的齿槽强度或齿槽距离设置到较低数值时尤甚。

我们推荐的电机无疑是迄今为止最顺滑（也就是说齿槽效应最小）的电机，也是现今唯一一种
适合此种应用的电机。

如果你发现了其他在断电时旋转顺滑的电机，还请你在 Discord 中与我们分享；而如果你想知道某款便宜电机
能不能用，也可以去 Discord 里问一问，不过你要做好被人说
“不行”的心理准备。

### 大致预算是多少？

不是我不想说，是我确实不知道。过段时间来看可能就有了——因为要不断修复、改进原型，所以我到现在为止一共也没做出来几个，核算成本就更别提了。各种零件加起来应该在两百刀以内？不过有些东西可能会涨价，而且有些商家还有最低起订量和运费方面的限制。

### 这能搭配那啥一起用吗？

不一定，不管你说的那啥到底是哪啥。我到目前为止只为视频中演示的那些功能编写了足够的固件，总体来说可能还不足以将其与生产环境相结合。配置基本齿槽功能的 API 已经写好了，此外无他。在固件这方面还有大坑需要填。如果你能自己做一个，我倒是很乐意协助你使其兼容你想要的“那啥”！

### 我能买到套件或是成品吗？

应该不行吧？反正我自己现在是没打算开售。也倒不是我藏着掖着，主要是硬件太难搞了，而且这项目也只是我的业余爱好而已。

SmartKnob 是开源的，许可也相当宽松，所以照理讲谁都可以出售相关的套件、成品。如果确实有人开始卖 SmartKnob，请注意，*需要*
署名（当然了，如果你有心🙂，愿意给我一些[使用费、打赏、感谢](https://github.com/sponsors/scottbez1/)之类，我也不会拒绝）。

# 固件和软件
更多的固件和软件文档（以及入门指南）都可以在 [SmartKnob Firmware and Software Guide](https://paper.dropbox.com/doc/SmartKnob-firmware-and-software--Byho6npe9XvZLZLxJ_8bK5TqAg-VUb9nq7btuhnHw5KlgJIH#:h2=Calibration)处找到

## 部件信息总览

### 磁编码器

#### MT6701 (MagnTek)（上海麦歌恩）
价位合适，品质上乘——强烈推荐。其噪声相较于 TLV493D 更小，使用 SSI 接口时的响应也更快（控制回路也更加稳定）。

 - 支持 SSI、I2C 和 ABZ 等多种接口——响应延迟应该不错
 - SSI 包含 CRC 数据校验
 - 没有省电模式或低功耗选项——可能并不适合电池供电设备
 - 美国国内没有经销商（Mouser 和 Digi-Key 上没有售卖）

[参数表](http://www.magntek.com.cn/upload/MT6701_Rev.1.5.pdf)

[于 LCSC 订购](https://lcsc.com/product-detail/Angle-Linear-Position-Sensors_Magn-Tek-MT6701CT-STD_C2856764.html)

#### TLV493D (Infineon)（英飞凌）
这种磁编码器虽然十分常见，却没放在 SmarKnob 总览里。因为如果要把它用在触觉反馈项目上，其实不太出彩。利用 [Adafruit 产的 QWIIC 分线板](https://www.adafruit.com/product/4366)可以很容易做出原型产品来。

我测试发现，它噪声现象很严重，需要滤波或者降噪，但这样又可能会减慢响应速度，降低控制回路的稳定性。而如果滤波不够，噪声
很容易就会被 PID 电机转矩控制器中的微分单元放大，从而导致听觉（和触觉）上的嗡嗡声（震动感）。

此外，它的芯片还有个已知问题，有时会导致内部的模数转换器完全锁死，必须完全重置、重新配置才能恢复，这可能会导致数据的延迟或丢失！详情请见[用户手册](https://www.infineon.com/dgdl/Infineon-TLV493D-A1B6_3DMagnetic-UM-v01_03-EN.pdf?fileId=5546d46261d5e6820161e75721903ddd)中的
第五章第六节。

    在主控模式(MCM)或快速模式(FM)下，模数转换器可能会直接宕机。宕机能够
    以下述方式检测：
     - 帧计数器（FRM）数值固定，不再增加。

我测试了四个不同的 Adafruit 分线板，其中两个（50%）会在运行一两分钟之后定期出现这种锁死情况。可以利用自动检测、自动重置来曲线救国（项目里也有相关的代码），但传感器经常锁死可能会导致意外的跳帧或延迟。

[参数表](https://www.mouser.com/datasheet/2/196/Infineon_TLV493D_A1B6_DataSheet_v01_10_EN-1227967.pdf)


#### AS5600(AMS)（艾迈斯）
普普通通，分线板便宜还不缺货。

它噪声现象比 TLV493D 还要严重，更需要滤波或者降噪，但如此可能会减慢响应速度，降低控制回路的稳定性。而且相较于我测试过的其他传感器，AS5600 更容易达到磁饱和状态，当搭配如 [Radial Magnets 的8995型号](https://www.digikey.com/en/products/detail/radial-magnets-inc/8995/5126077)磁铁时，需要较宽的间隙（8-10 mm）。

[参数表](https://ams.com/documents/20143/36005/AS5600_DS000365_5-00.pdf)

### 电机驱动器
#### TMC6300-LA
这是一款较新的电机驱动 IC，完美适合这个项目！一般其他（集成了场效应管的）驱动器都无法满足本项目中所使用的低电压、小电流电机的要求（DRV8316 应该也行，但还没测试）。

亮点：
 - 2-11V 直流电机输入
 - 有效电流高达 1.2A
 - 小巧（3x3mm QFN 封装）

[参数表](https://www.trinamic.com/fileadmin/assets/Products/ICs_Documents/TMC6300_datasheet_rev1.07.pdf)

[产品页面](https://www.trinamic.com/products/integrated-circuits/details/tmc6300-la/)

### 电机
#### 32mm 转子，空心轴，径向磁铁
<a href="doc/img/motors/PXL_20220121_221746595.jpg"><img src="doc/img/motors/PXL_20220121_221746595.jpg" width="200" /></a>
<a href="doc/img/motors/PXL_20220121_221738745.jpg"><img src="doc/img/motors/PXL_20220121_221738745.jpg" width="200" /></a>


 - 32mm 转子
 - 全高 15mm（含磁铁）或 12.75mm（不含磁铁），转子高度 9mm
 - 齿槽效应强度约等于零——能实现极致平滑的输入
 - 5.9mm 的空心轴
 - 内置径向磁铁，可搭配磁编码器使用
 - 已经过测试

整体来说算是最好上手的电机。齿槽效应不明显，而且还内置了径向磁铁，太完美了！

[SparkFun](https://www.sparkfun.com/products/20441) 上有售！

# 声明
此项目受到 Jesse Schoch 的视频“[haptic textures and virtual detents](https://www.youtube.com/watch?v=1gPQfDkX3BU)”和
[SimpleFOC 社区中的有关讨论](https://community.simplefoc.com/t/haptic-textures/301)。真的，如果没有那个视频，就不会出现这个项目了。谢谢你 Jesse！


# 许可

This project is licensed under Apache v2 (software, electronics, documentation) and Creative Commons Attribution 4.0 (hardware/mechanical) (see [LICENSE.txt](LICENSE.txt) and [Creative Commons](https://creativecommons.org/licenses/by/4.0/)).

    Copyright 2022 Scott Bezek
    
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    
        http://www.apache.org/licenses/LICENSE-2.0
    
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

