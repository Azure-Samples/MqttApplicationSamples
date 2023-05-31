from setuptools import setup, find_packages

setup(
    name="mqttclients",
    version="0.0.1", #desc, author
    install_requires=["python-dotenv", "paho-mqtt"],
    packages=find_packages()
)