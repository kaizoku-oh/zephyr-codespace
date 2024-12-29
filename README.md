# 🚀 Zephyr Codespace

Welcome to **Zephyr Codespace** – the ultimate GitHub template repository to kickstart your Zephyr RTOS projects effortlessly! 🎉

## 🤔 Why Zephyr Codespace?

Setting up the Zephyr toolchain can be a real pain 🛠️. But with **Zephyr Codespace**, you can:

- Get started quickly with **modern tools** like Docker and DevContainers 🐳.
- Skip the hassle of installing the Zephyr toolchain on your machine. Everything you need is baked into a **VS Code DevContainer**!
- Use **VS Code tasks** to build, clean, update, and manage your project with a single click ⚡.
- Debug your project like a pro using the **VS Code Cortex-Debug extension** 🐞.
- Seamlessly work locally or in GitHub Codespaces 🌐.

## ✨ Features

- **🔧 Fully Configured DevContainer:** Powered by the official Zephyr Docker image: `ghcr.io/zephyrproject-rtos/zephyr-build:latest`.
- **📦 Out-of-the-box OTA Updates and Event Management:** Optional services to enhance your Zephyr projects.
- **🖥️ VS Code Integration:** Pre-configured tasks for building, cleaning, and more.
- **🐞 Cortex-Debug Support:** Debug your Zephyr project with ease.
- **🚀 GitHub Codespaces Ready:** Start coding directly in the cloud or locally with Docker and VS Code.

## 💡 How to Use It

1. **Clone or Use Template**
   - Click on **"Use this template"** on GitHub.
   - Clone your new repository locally:
```bash
     git clone https://github.com/kaizoku-oh/zephyr-codespace.git
     cd zephyr-codespace
```

2. **Open in VS Code**
   - Open the project folder in VS Code.
   - When prompted, reopen in the DevContainer. If not prompted, use `F1 > Dev Containers: Reopen in Container`.

3. **Run Tasks**
   - Use the `Run Task` command in VS Code to:
     - 🛠️ Build your project.
     - 🧹 Clean your build directory.
     - 🔄 Update your dependencies.

4. **Debug Your Code**
   - 📥 Install the Cortex-Debug extension in VS Code.
   - 🐞 Configure your debug settings and start debugging.

5. **Deploy OTA Updates** *(Optional)*
   - Use the provided services for managing OTA updates and events.

## ✅ Requirements

- 🐳 **Docker**
- 🖥️ **Visual Studio Code**
- 🧩 **DevContainers Extension** (optional but recommended for local use)

## 🏠 Local Development

You can run this project locally on any machine that has Docker and VS Code installed. No Zephyr toolchain required! 🥳

## 🌐 Codespaces Development

Want to skip local setup entirely? Open this project directly from your browser using **GitHub Codespaces** and start coding instantly! 🚀

## 📜 License

This project is licensed under the **MIT License**. See the [LICENSE](LICENSE) file for more details.

## 🤝 Contributing

Contributions are welcome! Feel free to open issues, submit pull requests, or share feedback. Let’s make Zephyr development easier together! 💪

## 🙏 Acknowledgments

- **Zephyr Project** for their awesome project.
- **VS Code** and its amazing ecosystem.
- **Cortex-Debug Extension** for simplifying embedded debugging.
- **Golioth** for the directory structure and the helpful blog posts.

---

Happy Coding! 💻✨
