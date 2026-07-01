# 【重要】Windows VS Code C++ 開発環境構築ガイド

VRプログラミング実習のための開発環境を、各自の Windows PC に構築します。下記の手順を上から順に実施してください。各ツールが何のためのものかも併記しています。

## やることリスト
- **MSYS2** のインストールと、ツールチェーン（GCC/G++/GDB）の導入
- **MSYS2 経由でインストール**：OpenCV / OpenGL関連（glew・freeglut・Qt）
- **環境変数（PATH）の設定**
- **VS Code** と **C/C++ Extension Pack** の導入
- **Git for Windows** の導入と、配布プロジェクトのクローン
- **動作確認**：プロジェクトを開いて実行

> **注意：プロジェクトはクラウド同期フォルダ（OneDrive / Google Drive / iCloud 等）に置かないでください。** 同期がファイルを古い版に巻き戻し、編集が消える事故が起きます。`C:\Dev\` などローカルに置いてください。

---

## 手順1. MSYS2（Unix風の開発環境）
Windows 上に Unix 風の開発環境を構築するツールです。公式サイトからダウンロードしてインストールします。
- https://www.msys2.org/
- インストール先は**デフォルトの `C:\msys64`** を指定してください。

## 手順2. MSYS2 MINGW64 を起動・更新
1. スタートメニューから **MSYS2 MINGW64** を開きます。
2. 最初にパッケージを更新します。
   ```
   pacman -Syu
   ```
   - `Proceed with installation? [Y/n]` と聞かれたら `y` + Enter。
   - `terminal will be closed.` と表示されたら `y` + Enter で一度ウィンドウが閉じます。**再度 MINGW64 を開いてください。**

## 手順3. 開発ツール（GCC / G++ / GDB）
ツールチェーンをインストールします。
```
pacman -S mingw-w64-x86_64-toolchain
```
- デフォルトの `all` を選びたいので Enter、続けて `y` + Enter。以降、何か聞かれたらすべて `y`。
- これで **GCC / G++ / GDB** が入ります。

## 手順4. OpenCV（画像処理ライブラリ）
```
pacman -S mingw-w64-x86_64-opencv
```
デフォルトの `all` を選びたいので Enter で進めます。

## 手順5. OpenGL 関連（描画ライブラリ）
3Dグラフィックス描画に使います。以下を1つずつ順に実行します（聞かれたら Enter / `y` + Enter で進めます）。
```
pacman -S mingw-w64-x86_64-glew
```
```
pacman -S mingw-w64-x86_64-freeglut
```
```
pacman -S mingw-w64-x86_64-qt6
```

## 手順6. インストール確認
MINGW64 ターミナルで以下を実行し、どちらもバージョンが表示されれば成功です。
```
g++ --version
```
```
pkg-config --modversion opencv4
```

## 手順7. 環境変数（PATH）の設定
1. 「設定 → システム → バージョン情報 → システムの詳細設定」を開く。
2. 「システムのプロパティ」→「環境変数」→「Path」を選択して「編集」。
3. 「新規」で以下を追加し、「OK」で閉じる。
   ```
   C:\msys64\mingw64\bin
   ```
4. **Windows を再起動**します。
5. 確認：コマンドプロンプトで `gcc --version` を実行し、バージョンが表示されれば成功です。

## 手順8. VS Code（統合開発環境）
講義で使用する IDE です。
1. 公式サイトから Windows 版をダウンロードしてインストールします。
   - https://code.visualstudio.com/
2. （任意）日本語化：左の **Extensions** で「Japanese Language Pack」を検索・インストールし、Restart。

## 手順9. Git のインストールとプロジェクトの取得（GitHub から）
Windows には Git が標準で入っていないため、まず **Git for Windows** をインストールします（バージョン管理ツール。`git clone` や VS Code のソース管理に使います）。
- https://git-scm.com/download/win
- ダウンロードしたインストーラを実行し、基本的に既定のまま進めて構いません。
- **インストール後は、新しい PowerShell ウィンドウを開いてください**（PATH を反映させるため）。確認：`git --version` でバージョンが表示されればOK。
- 以降の操作は **PowerShell** で行ってください（コマンドプロンプトでは git が使えないことがあります。本プロジェクトのビルド/実行タスクも PowerShell を使います）。

配布リポジトリを**ローカルディスク**にクローンします（クラウド同期フォルダ不可）。`C:\Dev` などの作業フォルダを作って移動し、clone します。以下を**1つずつ**実行してください。

> PowerShell では複数コマンドを `&&` で繋げられません（`&&` は PowerShell 7 以降のみ）。繋げたい場合は `;` を使うか、下記のように1コマンドずつ実行してください。

```
mkdir C:\Dev -Force
```
```
cd C:\Dev
```
```
git clone https://github.com/Nozomi-Nishiumi/DXS_win_vscode_distribution.git
```
VS Code の **File → Open Folder** でクローンしたフォルダを開きます。作成者を信頼するか聞かれたら **Yes**。

## 手順10. C++ 開発環境用の拡張機能
画面左の **Extensions** をクリックし、**「C/C++ Extension Pack」** を検索してインストールします。

## 手順11. 設定（ほぼ自動）
- インクルードパス等は本リポジトリ同梱の `.vscode/c_cpp_properties.json`（`Windows-MSYS2` 構成）で設定済みです（C standard: c17 / C++ standard: c++17）。
- 赤波線（IntelliSense の誤検知）が出る場合は、Ctrl+Shift+P →「**C/C++: Reset IntelliSense Database**」、または VS Code を再起動してください。ビルド・実行には影響しません。

## 手順12. 動作確認
1. **Ctrl+Shift+B（Run Build Task）** を押します。既定タスク「Run OpenGL project」がビルド→実行まで自動で行います（PowerShell 上で動作し、PATH に `C:\msys64\mingw64\bin` を一時追加します）。
2. ターミナルに **`Select mode (1–7):`** と表示されれば**環境構築成功**です。番号を入力すると各モードが実行されます。

## 手順13. カメラの許可
`camera not found` などのエラーが出る場合は、Windows の設定で以下3箇所をオンにしてください。
- カメラへのアクセス
- アプリにカメラへのアクセスを許可する
- デスクトップアプリがカメラにアクセスできるようにする

---

## デバッグについて（cppdbg + gdb）
「実行とデバッグ」から **Debug OpenGLApp (Windows MSYS2)** を起動します（MSYS2 の `gdb.exe` を使用、外部コンソールで動作）。

## モードと教材の流れ
| # | 内容 |
|---|---|
| 1 | OpenGL の基礎 |
| 2 | 静止画の色トラッキング |
| 3 | カメラ接続確認 |
| 4 | カメラのライブトラッキング基礎 |
| 5 | 色しきい値をトラックバーで調整（`output/cam0_*.yml` に自動保存） |
| 6 | カメラ較正 → `output/cam_intrinsic_prameters_test.yml` を出力 |
| 7 | PnP による VR 表示（mode 5・6 の結果を読み込み。`v` キーで視点切替） |

3Dモデル（.obj/.mtl）は `common_data/CG_objects/` に、実行時生成ファイルは `DX_studies/output/` に置かれます。

## トラブルシューティング
- **`opencv2/opencv.hpp` や `GL/glut.h` が見つからない** → 手順4・5の MSYS2 パッケージが未導入。
- **`g++.exe` / `gdb.exe` が見つからない** → `C:\msys64\mingw64\bin` の存在を確認。MSYS2 を別の場所に入れた場合は `.vscode/` 内の各パスを合わせる。
- **実行時に DLL エラー** → PATH に `C:\msys64\mingw64\bin` を追加（手順7。ビルド/実行タスクは自動で追加します）。
- **編集が勝手に元に戻る** → クラウド同期フォルダに置いている可能性。ローカルへ移動。

> ソースコード（`DX_studies/`・`common_functions/`・`common_data/`）は Mac 版配布と同一内容です。プラットフォーム依存は `.vscode/` 設定のみです。
