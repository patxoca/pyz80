from setuptools import setup, find_packages, Extension


z80emu = Extension(
    "pyz80",
    sources=["src/pyz80module.c"],
    extra_objects=["src/z80emu/z80emu.a"],
)

setup(
    name="pyz80",
    version="0.1.0",
    description="Bindings for the Z80 emulator.",
    license="GPL",
    packages=find_packages(exclude=["ez_setup"]),
    namespace_packages=[],
    ext_modules=[z80emu],
    zip_safe=False,
    install_requires=[
        "setuptools",
    ],
    entry_points={},
)
