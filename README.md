<div align="center">

# 🚀 PyForge IDE

**Un IDE Python moderne et performant écrit en C++ / Qt 6**

![Version](https://img.shields.io/badge/version-3.0-blue.svg)
![Platform](https://img.shields.io/badge/platform-Windows-lightgrey.svg)
![Qt](https://img.shields.io/badge/Qt-6.11-41cd52.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)

</div>

## ✨ Fonctionnalités

- 🎨 **Interface moderne** - Dark theme professionnel (4 thèmes)
- 🐍 **Coloration syntaxique Python** - Lexer C++ ultra-rapide
- 💡 **Auto-complétion intelligente** - Mots-clés, built-ins, snippets
- ⌨️ **Terminal interactif** - Historique, auto-complétion Tab
- 📁 **Explorateur de fichiers** - Avec icônes par type
- 🎯 **Palette de commandes** - Style VS Code (Ctrl+Shift+P)
- 🎨 **Formatage automatique** - Intégration Black
- 🔍 **Linter intégré** - Support Ruff
- ⚡ **Léger et rapide** - ~50MB RAM, démarrage <1s
- 💾 **Sauvegarde de session** - Reprend là où vous vous êtes arrêté

## 📦 Installation

### Prérequis

- Windows 10/11 (64-bit)
- Python 3.10+ ([Télécharger](https://www.python.org/))

### Outils optionnels

```bash
pip install black ruff
```

## 🛠️ Compilation depuis les sources

### Prérequis pour compiler

- Visual Studio 2022 Build Tools
- Qt 6.11+ (msvc2022_64)
- CMake 3.20+
- Ninja

### Build

```bash
git clone https://github.com/TON_USER/PyForgeIDE.git
cd PyForgeIDE

cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="C:/Qt/6.11.1/msvc2022_64"

cmake --build build --config Release
```

## ⌨️ Raccourcis clavier

| Raccourci | Action |
|-----------|--------|
| `Ctrl+N` | Nouveau fichier |
| `Ctrl+O` | Ouvrir fichier |
| `Ctrl+S` | Sauvegarder |
| `F5` | Exécuter |
| `Ctrl+Shift+P` | Palette de commandes |
| `Ctrl+/` | Commenter |
| `Ctrl+D` | Dupliquer ligne |
| `Ctrl+G` | Aller à la ligne |
| `Shift+Alt+F` | Formater (Black) |
| `Ctrl+Space` | Auto-complétion |

## 🏗️ Architecture

```
PyForgeIDE/
├── src/
│   ├── main.cpp
│   ├── core/          # Moteur (SyntaxHighlighter, etc.)
│   ├── ui/            # Interface (MainWindow, Editor, Terminal)
│   └── utils/         # Utilitaires (Theme)
├── resources/         # Icônes, thèmes
└── CMakeLists.txt
```

## 🎯 Roadmap

- [ ] Débogueur Python intégré
- [ ] Support Git
- [ ] Minimap
- [ ] Support Jupyter Notebooks
- [ ] Extensions/Plugins

## 📄 Licence

Distribué sous licence MIT.

---

<div align="center">
Fait avec ❤️ par Bénéwendé Dieudonné Tassembédo en C++ et Qt 6
</div>