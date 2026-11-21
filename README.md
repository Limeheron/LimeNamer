# LimeNamer

여러 파일과 폴더의 이름을 한 번에 바꾸는 프로그램이다. Windows와 macOS를 지원한다.

## 주요 기능

- 넣기, 지우기, 바꾸기, 형식 맞추기를 여러 개 쌓아 차례로 적용한다.
- 번호, 날짜, 파일 크기 같은 값을 이름에 넣을 수 있다.
- 바뀔 이름을 미리 보여 주고, 쓸 수 없거나 겹치는 이름은 실행 전에 알려 준다.
- 마지막으로 바꾼 이름을 되돌릴 수 있다.

## 설치법

### Windows

Windows 11 이상과 .NET 데스크톱 런타임 10이 필요하다. 런타임은 PowerShell이나 터미널에서 아래 명령으로 설치하거나, [.NET 10 다운로드 페이지](https://dotnet.microsoft.com/download/dotnet/10.0)에서 받는다.

```
winget install Microsoft.DotNet.DesktopRuntime.10 --architecture x64
```

1. [릴리스](https://github.com/Limeheron/LimeNamer/releases)에서 LimeNamer-버전-win-x64.zip을 받는다.
2. 원하는 곳에 압축을 풀고 LimeNamer 폴더의 LimeNamer.exe를 실행한다. exe 파일만 옮기면 실행되지 않으므로, 다른 곳에서 열려면 바로가기를 만든다.

지울 때는 LimeNamer 폴더를 삭제한다. 설정은 `%LOCALAPPDATA%\LimeNamer`에 따로 남는다.

### macOS

macOS 14 이상의 Apple Silicon Mac이 필요하다.

1. [릴리스](https://github.com/Limeheron/LimeNamer/releases)에서 LimeNamer-버전-mac-arm64.dmg를 받아 열고, LimeNamer를 응용 프로그램 폴더로 끌어다 놓는다.
2. 처음 열 때 열 수 없다는 안내가 뜨면, 시스템 설정의 개인정보 보호 및 보안에서 "그래도 열기"를 누른다.

지울 때는 LimeNamer를 휴지통으로 옮긴다. 설정은 `~/Library/Preferences/io.github.limeheron.limenamer.plist`에 따로 남는다.
