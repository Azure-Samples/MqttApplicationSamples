from setuptools import setup, find_packages

setup(
    name="connectionsettings",
    version="0.0.1", #desc, author
    install_requires=["python-dotenv", "paho-mqtt"],
    packages=find_packages()
)