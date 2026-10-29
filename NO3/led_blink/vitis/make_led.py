# Vitis 2026.1 script: HW2 assignment 1 (LED blink)
import vitis, re, os, shutil

ROOT = "C:/vitis_led"            # xsa + scripts live here
WS   = ROOT + "/ws"              # fresh Vitis workspace (created by Vitis)
XSA  = ROOT + "/design_1_wrapper.xsa"

client = vitis.create_client()
client.set_workspace(path=WS)

# 1. Platform
if not os.path.exists(WS + "/led_platform"):
    print(">>> [1/4] Creating platform ...")
    plat = client.create_platform_component(
        name="led_platform", hw_design=XSA,
        os="standalone", cpu="ps7_cortexa9_0")
    plat.build()
else:
    print(">>> [1/4] Platform already exists, skip")

# 2. Application (empty template: the design has no UART)
if not os.path.exists(WS + "/led_blink"):
    print(">>> [2/4] Creating application ...")
    app = client.create_app_component(
        name="led_blink",
        platform=WS + "/led_platform/export/led_platform/led_platform.xpfm",
        domain="standalone_ps7_cortexa9_0",
        template="empty_application")
else:
    print(">>> [2/4] Application already exists, skip")
    app = client.get_component(name="led_blink")

# 3. main.c + run from on-chip memory (OCM)
shutil.copyfile(ROOT + "/main_led.c", WS + "/led_blink/src/main.c")
ld = WS + "/led_blink/src/lscript.ld"
txt = open(ld, encoding="utf-8").read()
txt = re.sub(r">\s*ps7_ddr_0", "> ps7_ram_0", txt)
open(ld, "w", encoding="utf-8").write(txt)
print(">>> [3/4] main.c copied, lscript.ld -> ps7_ram_0")

# 4. Build
print(">>> [4/4] Building application ...")
app.build()
vitis.dispose()
print(">>> ALL DONE: " + WS + "/led_blink/build/led_blink.elf")
