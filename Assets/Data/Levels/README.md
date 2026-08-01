# Maze Level Data

파일 미로와 시드 기반 절차적 미로를 하나의 레벨 목록에서 진행 순서대로 관리한다. 두 소스는 모두 `MazeDefinition`을 만들며 이후 렌더링, 충돌, 출구와 적 스폰 흐름을 공유한다.

## 파일 미로 형식

각 텍스트 파일은 한 행이 미로의 한 행에 대응하는 직사각형 격자다.

| 문자 | 의미 | 개수 |
|---|---|---:|
| `*` | 벽 | 제한 없음 |
| `.` | 이동 가능한 통로 | 제한 없음 |
| `P` | 플레이어 시작점 | 정확히 1개 |
| `X` | 출구 | 정확히 1개 |
| `@` | 안내판 | 0개 이상 |

- 파일은 UTF-8 BOM 없이 저장한다.
- 빈 통로를 공백으로 표현하지 않는다.
- 첫 행은 비어 있을 수 없다.
- 모든 행의 문자 수는 첫 행과 같아야 한다.
- 허용된 문자 외의 값은 사용할 수 없다.
- 좌표는 왼쪽 위가 `(row: 0, column: 0)`이다.
- 적 위치는 파일 마커가 아니라 레벨 목록의 수량·시드로 결정한다.

로더는 마커 좌표를 `MazeDefinition`에 저장한 뒤 해당 셀을 통로인 `.`으로 정규화한다.

## 레벨 목록 형식

`LevelList.txt`는 게임에서 사용할 레벨을 진행 순서대로 관리한다.

```text
# file|path|enemyCount|enemySpawnSeed
# procedural|passageRows|passageColumns|mazeSeed|enemyCount|enemySpawnSeed
file|Assets\Data\Levels\Level01.txt|1|1001
procedural|8|10|3001|3|3002
```

### 파일 레벨

```text
file|path|enemyCount|enemySpawnSeed
```

- `path`: 저장소 루트 기준 미로 파일 경로
- `enemyCount`: 생성할 적 수인 0 이상의 정수
- `enemySpawnSeed`: 적 위치를 재현하는 부호 없는 정수
- 같은 파일 경로를 중복 등록할 수 없다.
- 기존 `path|enemyCount|enemySpawnSeed` 형식도 호환을 위해 읽을 수 있다.

### 절차적 레벨

```text
procedural|passageRows|passageColumns|mazeSeed|enemyCount|enemySpawnSeed
```

- `passageRows`, `passageColumns`: 벽을 제외한 논리적 통로 셀 수
- 실제 격자 크기: 각 축에서 `2 * passageCount + 1`
- `mazeSeed`: 같은 미로 구조를 재현하는 부호 없는 정수
- 행·열·미로 시드가 모두 같은 절차적 항목은 중복으로 거부한다.
- 반복형 깊이 우선 탐색으로 모든 통로를 연결하고, 시작점에서 가장 먼 논리 셀을 출구로 사용한다.

빈 줄과 `#`으로 시작하는 주석은 무시하며, 첫 번째 유효 항목이 게임 시작 레벨이다.
