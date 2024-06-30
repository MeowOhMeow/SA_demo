# Installation
- On Windos
  - You'll need to install the following software:
    - [gnuplot](http://www.gnuplot.info/download.html) <br>
      Note that you'll need to add the gnuplot installation directory to the system path.
    - [MSYS2](https://www.msys2.org/)
    - make for windows. One Option is to use [chocolatey](https://chocolatey.org/). You can install it by running the following command in terminal:
      ```bash
      choco install make
      ```
      Note that you'll need to run the terminal as an administrator.
  - After installing the above software, you can run the following command to install the required dependencies:
    - In the MSYS2 terminal, run the following command:
      ```bash
      pacman -S --noconfirm --needed mingw-w64-x86_64-toolchain mingw-w64-x86_64-glfw
      ```

- On Linux
  ```bash
  sudo apt-get update
  sudo apt-get install build-essential make libglfw3-dev libglm-dev
  sudo apt-get install gnuplot-x11
  ```

# Building
- Run the following command in the terminal:
  ```bash
  cd src
  make
  ```

# Running
- Run the following command in the terminal:
  - on Windows
    ```bash
    ./SA_demo.exe
    ```
  - on Linux
    ```bash
    ./SA_demo
    ```
