## Repository of projects made in Computer Engineering graduation.

This reposotory uses Arduino-CLI

### How to install Arduino-CLI

### How to install Esp32 boards

### How to compile and run a code

- Picking board COM:

`arduino-cli board list`

Pick your board COM

- Picking board fqbn:

`arduino-cli board search`

Pick your board fqbn key.

- Compile:

Run: `arduino-cli compile --fqbn [your_board_fqbn] /project_file_path`

- Upload:

Run: `arduino-cli upload -p [your_board_COM] --fqbn [your_board_fqbn] /project_file_path`