# Initialize the west repository in the 'app' subdirectory
if [ ! -d ".west" ]; then
    echo "[setup.sh] Initializing the west repository in the 'app' subdirectory"
    west init -l app
fi

# Make sure the workspace contains Git repositories matching the projects in the manifest file
echo "[setup.sh] Updating west repository"
west update

# Export Zephyr-specific build system metadata to the 'app' directory
echo "[setup.sh] Exporting Zephyr-specific build system metadata to the 'app' directory"
west zephyr-export

# Install Python dependencies required by Zephyr
echo "[setup.sh] Installing Python dependencies required by Zephyr"
pip install -r deps/zephyr/scripts/requirements.txt

# Copy .vscode to outer directory
echo "[setup.sh] Copying .vscode to outer workspace directory"
if [ -d ".vscode" ]; then
    echo "[setup.sh] .vscode already exists in outer workspace directory"
else
    cp -r app/.vscode .
fi

# Download and install Fira Code
echo "[setup.sh] Downloading and installing Fira Code font"
fonts_dir="${HOME}/.fonts"
mkdir -p "${fonts_dir}"
version=5.2
zip=Fira_Code_v${version}.zip
wget --no-verbose https://github.com/tonsky/FiraCode/releases/download/${version}/${zip} -O ${zip}
unzip -o -q -d "${fonts_dir}" "${zip}"
rm "${zip}"

echo "fc-cache -f"
fc-cache -f
