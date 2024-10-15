To build this project from source, you'll need to have vcpkg installed and added to your system's PATH environment variable. Here's how you can do it:

1. **Download vcpkg**: 
   - You can download vcpkg from its GitHub repository: [Microsoft/vcpkg](https://github.com/microsoft/vcpkg):
    ```
    git clone https://github.com/Microsoft/vcpkg.git
    ```
   - Setup executable file:
    ```
    cd vcpkg
    ./bootstrap-vcpkg.sh # bootstrap-vcpkg.bat for Powershell
    ./vcpkg integrate install
    ```
    At this point, vcpkg is set up and ready to use


2. **Add vcpkg to PATH**:
   - Add the directory containing the vcpkg executable to your system's PATH environment variable. This step allows you to run vcpkg from any directory in your command prompt or terminal.

3. **Install dependencies using vcpkg**:
   - Once vcpkg is set up, open a command prompt or terminal.
   - Navigate to this project folder ( Same location with vcpkg-configuration.json file ).
   - Run the following command to install dependencies using vcpkg:
     ```
     vcpkg install
     ```
   This command will download all dependencies.

After following these steps, you should be able to build the project successfully.

4. Build project:
   1. Install deps:
   - For Linux: run this before generating cmake files
      ```
      sudo apt install libglu1-mesa-dev
      ```
   - For Mac: run this before generating cmake files
      ```
      brew install pkg-config
      ```
   2. Build
   - Create folder build
      ```
      mkdir build
      cd build
      ```

   - Run this command: Change "default" to Linux/Mac - depends on your system.
     ```
     cmake .. --preset=default
     ```

   - After create cmake file successfully, there is a build folder, go ther and run:
     ```
     cmake --build . --config Release --parallel
     ```
   
   - Run:
     ```
     ./uvgVisualzier
     ```

   **Note: If there are any issue related to rendering in OpenGL, check if there is a folder named as "shaders" at the same place with the visualizer. (Copy it if needed) 


