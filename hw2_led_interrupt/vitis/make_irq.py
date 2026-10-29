# Vitis 2026.1 script: HW2 assignment 2 (button interrupt)
import vitis, re, os, shutil

ROOT = "C:/vitis_irq"            # xsa + scripts live here
WS   = ROOT + "/ws"              # fresh Vitis workspace (created by Vitis)
XSA  = ROOT + "/design_1_wrapper.xsa"

client = vitis.create_client()
client.set_workspace(path=WS)

# 1. Platform
if not os.path.exists(WS + "/irq_platform"):
    print(">>> [1/4] Creating platform ...")
    plat = client.create_platform_component(
        name="irq_platform", hw_design=XSA,
        os="standalone", cpu="ps7_cortexa9_0")
    plat.build()
else:
    print(">>> [1/4] Platform already exists, skip")

# 2. Application
if not os.path.exists(WS + "/led_irq"):
    print(">>> [2/4] Creating application ...")
    app = client.create_app_component(
        name="led_irq",
        platform=WS + "/irq_platform/export/irq_platform/irq_platform.xpfm",
        domain="standalone_ps7_cortexa9_0",
        template="empty_application")
else:
    print(">>> [2/4] Application already exists, skip")
    app = client.get_component(name="led_irq")

# 3. main.c (copied from main_irq.c) + run from on-chip memory
shutil.copyfile(ROOT + "/main_irq.c", WS + "/led_irq/src/main.c")
ld = WS + "/led_irq/src/lscript.ld"
txt = open(ld, encoding="utf-8").read()
txt = re.sub(r">\s*ps7_ddr_0", "> ps7_ram_0", txt)
open(ld, "w", encoding="utf-8").write(txt)
print(">>> [3/4] main.c copied, lscript.ld -> ps7_ram_0")

# 4. Build
print(">>> [4/4] Building application ...")
app.build()
vitis.dispose()
print(">>> ALL DONE: " + WS + "/led_irq/build/led_irq.elf")
