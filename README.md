# RedWill OS

OS i386 « fait maison » : boot BIOS, noyau 32 bits, interface graphique VBE, et maintenant une **API RedWill**
avec des scripts `.rpp` et des exécutables `.rxe`.

## Compiler l'OS

Prérequis (Linux) : `gcc` avec le support 32 bits (`gcc-multilib`), `binutils`, `python3` + `Pillow` (`pip install pillow`).

```sh
./build.sh          # -> build/redwill.iso   (et build/rppc, le compilateur de scripts pour ton PC)
./build.sh 8        # idem, avec 8 Mo de remplissage dans l'ISO (pour les tests USB)
qemu-system-i386 -m 128 -cdrom build/redwill.iso      # test (au moins 64 Mo de RAM)
```

`build.sh` fait dans l'ordre : compile `rppc` → convertit `inter32/icons/*.ico` → compile chaque `.rpp` de `rootfs/`
en `.rxe` et empaquette tout `rootfs/` dans le disque RAM `C:\` → compile le noyau → fabrique l'ISO.

## Arborescence du projet

| Dossier | Contenu |
|---|---|
| `boot/` | secteur de boot (El Torito), MBR hybride, script de link |
| `kernel/` | noyau : `arch/`, `gfx/`, `drivers/`, `fs/` (système de fichiers à dossiers), `ui/`, `rpp/` (langage + API), `main.c` |
| `userland/` | terminal, calculatrice, snake, navigateur |
| `rootfs/` | **ce qui apparaît dans `C:\` au démarrage** (voir ci-dessous) |
| `inter32/icons/` | icônes `.ico` 32x32 (régénérables avec `tools/gen_icons.py`) |
| `tools/` | `rppc.c` (compilateur), `mkrootfs.py`, `ico2c.py`, `gen_icons.py`, `gen_font.py`, `mkiso.py` |

## Disque C: de RedWill

```
C:\RedWill\red32          fichiers système (API.TXT = référence de l'API, SYSTEM.INI, SYSINFO...)
C:\RedWill\red-sdk        exemples de scripts (.rpp) et leurs exécutables (.rxe)
C:\RedWill\My Documents   tes fichiers
```
Pour ajouter un fichier ou un exemple dans l'OS : mets-le dans `rootfs/RedWill/...` et relance `./build.sh`.
Le disque est en RAM : ce qui est créé dans l'OS disparaît à l'extinction.

## Écrire un programme (langage RPP)

Un script `.rpp` est un petit langage proche du C. Exemple minimal (`red-sdk\window.rpp`) :

```
var win, lbl, count;
func clicked(btn) { count += 1; set_text(lbl, "Clique " + count + " fois"); }
win = window("Ma fenetre", 320, 150);
lbl = label(win, 20, 20, "Appuie sur le bouton");
button(win, 20, 60, 120, 32, "Clique", clicked);
```

L'API (fenêtres, boutons, zones de texte, listes, cases à cocher, dessin, minuteurs, clavier/souris, boîtes de dialogue,
fichiers…) est décrite dans **`C:\RedWill\red32\API.TXT`** (même texte que `rootfs/RedWill/red32/API.TXT`).

### Lancer / compiler *dans* RedWill

* Explorateur : double-clic sur un `.rxe` ou un `.rpp` le lance ; « Edit in Notepad » ouvre le code.
* Bloc-notes : bouton **Run** exécute le texte affiché ; **Open** / **Save** ouvrent la fenêtre d'explorateur (Enregistrer sous).
* Terminal :
  ```
  cd C:\RedWill\red-sdk
  run hello            lance hello.rxe (ou hello.rpp)
  rppc hello.rpp       compile -> hello.rxe       (rppc SRC [SORTIE.rxe])
  ```

### Compiler *sur ton PC*

```sh
cc -O2 -o rppc tools/rppc.c           # (build.sh le fait déjà : build/rppc)
./rppc prog.rpp                       # -> prog.rxe
./rppc prog.rpp -o autre/nom.rxe
./rppc --run prog.rpp                 # exécute sur le PC (programmes console uniquement)
```
Mets ensuite le `.rxe` dans `rootfs/RedWill/red-sdk/` (ou n'importe quel dossier de `rootfs/`) et refais `./build.sh`.
(Un `.rpp` placé dans `rootfs/` est compilé automatiquement en `.rxe` par `build.sh`.)

### Format `.rxe`
`"RXE1"`, puis ncode, nconst, nglobals, nfonctions (u32 little-endian), table des fonctions (adresse, nb params, nb locales),
code (mots 32 bits), constantes (chaînes). Machine à pile : voir `kernel/rpp/rpp.h`.

## Boîte « Enregistrer sous »
Bloc-notes → **Save** (ou Ctrl+S) ouvre un explorateur modal : navigue dans les dossiers (double-clic, « Up », « New dir »),
tape le nom, **Save**. Ctrl+O / **Open** ouvre un fichier. Les programmes RPP peuvent l'appeler avec `save_dialog()` / `open_dialog()`.
