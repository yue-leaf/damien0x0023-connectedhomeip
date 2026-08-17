import os
import shutil

if __name__ == "__main__":
    try:
        cwd = os.path.dirname(os.getcwd())
        ftl_v1_path = os.path.join(cwd, 'ftl_v1')
        ftl_v2_gcc_lib_path = os.path.join(cwd, 'bsp', 'sdk_lib', 'lib', 'rtl87x2g', 'gcc', 'librtl87x2g_sdk.a')
        ftl_v2_keil_lib_path = os.path.join(cwd, 'bsp', 'sdk_lib', 'lib', 'rtl87x2g', 'mdk', 'rtl87x2g_sdk.lib')
        ftl_v2_head_file_path = os.path.join(cwd, 'bsp', 'sdk_lib', 'inc', 'ftl.h')

        print("start update ftl_v2 to ftl_v1")
        print(".............................")
        shutil.copy2(os.path.join(ftl_v1_path, 'librtl87x2g_sdk.a'), ftl_v2_gcc_lib_path)
        shutil.copy2(os.path.join(ftl_v1_path, 'rtl87x2g_sdk.lib'), ftl_v2_keil_lib_path)
        shutil.copy2(os.path.join(ftl_v1_path, 'ftl.h'), ftl_v2_head_file_path)
        print("SUCCESS")
    except:
        print("Error,Please update manually!")

