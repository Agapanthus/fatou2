import os
from glob import glob
import pathlib
import subprocess

selfPath = pathlib.Path(__file__).parent.resolve()
for ext in ["vert", "frag"]:
    result = [
        y for x in os.walk(selfPath) for y in glob(os.path.join(x[0], "*." + ext))
    ]
    for x in result:
        subprocess.run(
            [
                "C:\\VulkanSDK\\1.3.216.0\\Bin\\glslc.exe",
                x,
                "-o",
                x[: x.rfind(".")] + "." + ext + ".spv",
            ]
        )
