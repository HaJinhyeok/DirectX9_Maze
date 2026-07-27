# 외부 에셋 및 라이선스

이 문서는 공개 저장소에 포함되는 모델과 텍스처의 출처, 이용 조건과 조치 상태를 기록한다. 프로젝트 코드와 문서에 적용되는 라이선스가 외부 에셋에 자동으로 적용되지는 않는다.

## 공개 기준

- 원저작자와 라이선스를 확인할 수 있는 에셋만 공개 저장소에 포함한다.
- 출처를 확인했더라도 원본 파일 재배포가 금지된 에셋은 자체 제작 또는 재배포 가능한 에셋으로 교체한다.
- 출처가 불명확한 에셋은 추정 출처를 확정 사실처럼 표기하지 않는다.
- 생성물이나 실행 화면에만 사용할 수 있는 에셋과 원본 파일을 함께 배포할 수 있는 에셋을 구분한다.

## 에셋 현황

| 파일 | 용도 | 확인된 출처와 근거 | 이용 조건 | 공개 판단 | 조치 |
| --- | --- | --- | --- | --- | --- |
| `Assets/Models/Tiger/*` | 적 모델과 텍스처 | Microsoft DirectX SDK June 2010의 `Samples/Media/Tiger`. `tiger.x`는 SDK 파일과 SHA-256이 같고 `tiger.bmp`는 모든 픽셀이 같다. | SDK EULA `2.a Media Elements and Templates`는 프로젝트 내 사용과 비상업적 프로젝트 배포를 허용한다. | 조건부 유지 | 외부 에셋으로 분리 표기하고 비상업적 포트폴리오 범위에서만 사용한다. 상업적 배포 전에는 별도 허가 또는 교체가 필요하다. |
| `Assets/Textures/Skybox/Daylight/*.bmp` | 현재 렌더링되는 스카이박스 | OpenGameArt의 [Sky Box - Sunny Day](https://opengameart.org/content/sky-box-sunny-day), 저자 Chad Wolfe, 요청 표기 `KIIRA` | [CC BY 3.0](https://creativecommons.org/licenses/by/3.0/) 선택 | 유지 가능 | 저작자와 라이선스 링크를 이 문서에 유지한다. |
| `SkyBox1.dds` | 사용되지 않던 큐브맵 | 확인되지 않음. 실제 렌더링은 `SkyBox`의 여섯 BMP를 사용했다. | 불명확 | 제거 완료 | 큐브맵·큐브 메시의 생성과 해제 코드를 정리하고 파일을 공개 저장소에서 제거했다. |
| `Assets/Textures/Environment/tex_wall.jpg` | 미로 벽 | 2026-07-27 프로젝트용으로 새로 생성하고 벽돌 주기 경계에 맞춰 재구성한 1024×1024 JPEG 이미지 | 프로젝트 제작 에셋 | 교체·검증 완료 | TilingTextures 원본 파일을 대체했다. 게임 내 밝기, 벽돌 크기와 반복 경계가 자연스러움을 확인했다. |
| `questions.png` | 사용되지 않던 원본 스프라이트 시트 | [The Spriters Resource의 Windows XP Question Mark](https://www.spriters-resource.com/pc_computer/windowsxpbuiltinapplications/asset/168068/)에서 확인한 Windows XP OOBE 추출 시트 | Microsoft 원저작물의 재배포 허가를 확인하지 못했다. | 제거 완료 | 공개 저장소에서 제거했다. |
| `Assets/Textures/UI/tex_question.png` | 안내판 아이콘 | 2026-07-27 프로젝트용으로 새로 생성하고 크로마키 제거와 축소 후 적용한 256×256 RGBA 이미지 | 프로젝트 제작 에셋 | 교체·검증 완료 | Windows XP 아이콘을 대체했다. 게임 내 선명도와 투명 배경을 확인했다. |
| `Assets/Textures/Environment/tex_grass.jpg` | 미로 바닥 | 2026-07-27 프로젝트용으로 새로 생성하고 밝기를 조정한 1024×1024 탑다운 잔디 JPEG 이미지 | 프로젝트 제작 에셋 | 교체·검증 완료 | 기존 출처 불명 잔디를 대체했다. 낮에는 질감이 보이고 밤에는 진행 경로를 읽을 수 있는 현재 밝기로 확정했다. |
| `Assets/Textures/Gameplay/tex_bullet.bmp` | 총알 구체 재질 | 2026-07-27 프로젝트용으로 새로 생성한 512×512 무채색 금속 BMP 이미지 | 프로젝트 제작 에셋 | 교체·검증 완료 | 기존 출처 불명 격자 이미지를 대체했다. 파일 이름도 실제 용도에 맞게 변경했다. |
| `Assets/Textures/Gameplay/tex_player_marker.bmp` | 탑뷰 플레이어 표식 구체 재질 | 프로젝트 제작 총알 재질에서 색상을 밝은 청록색으로 변환한 512×512 BMP 이미지 | 프로젝트 제작 파생 에셋 | 추가·검증 완료 | 총알과 플레이어 표식의 시각적 역할을 구분했다. |
| `Assets/Textures/UI/tex_exit.png` | 출구 표식 | 2026-07-27 프로젝트용으로 새로 생성하고 크로마키 제거, 정사각형 재배치와 축소 후 적용한 256×256 RGBA 이미지 | 프로젝트 제작 에셋 | 교체·검증 완료 | 기존 출처 불명 표식을 대체했다. 게임 내 방향, 크기와 투명 배경을 확인했다. |
| `docs/images/gameplay.png` | README 게임 화면 | 이 프로젝트의 실행 화면을 직접 캡처함 | 프로젝트 문서용 이미지 | 유지 | 공개 문서에 유지한다. |

## 출처 참고

- DirectX SDK June 2010: [Microsoft 공식 다운로드](https://www.microsoft.com/en-us/download/details.aspx?id=6812)
- DirectX SDK 설치본의 라이선스 원문: `Documentation/License Agreements/DirectX SDK EULA.txt`

## 검증 결과

- 불명확하거나 원본 재배포가 부적합한 이미지 에셋을 프로젝트 제작 에셋으로 교체하거나 제거했다.
- 유지하는 외부 에셋은 출처, 이용 조건과 공개 범위를 문서화했다.
- `Debug|x86` 빌드에서 새 `Assets` 경로와 플레이어 표식 텍스처가 정상적으로 컴파일·링크됨을 확인했다.
- 정리된 경로에서 런타임 모델과 텍스처가 정상 표시되고 플레이어 표식이 총알과 구분됨을 확인했다.
