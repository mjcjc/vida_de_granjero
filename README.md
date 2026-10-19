# Vida de Granjero | 멀티플레이 농장 게임의 서버 콘텐츠 처리

졸업작품의 서버 측 이동 처리, 월드 상태 전달, 계정·세션 관리를 정리하기 위한 검토 초안입니다. **클라이언트는 본인이 수정하지 않았으므로 개인 작업 소개에서 제외합니다.**

이전 초안에서 대표 경험으로 선정한 원격 플레이어 보간·외삽·회전 보간은 클라이언트 코드였습니다. 해당 내용을 본인의 구현 성과로 내세우지 않습니다. 서버 코드의 존재와 본인의 기여도도 별도로 확인해야 하므로, 아래는 담당 범위를 확인할 후보와 그 코드 근거입니다.

## 대표 주제 후보: 이동 요청을 검증하고 결과를 공유하는 서버 처리

**콘텐츠 관점:** 여러 플레이어의 이동 요청을 받아 게임 월드의 지형·장애물 조건에 따라 위치를 판단하고, 처리 결과를 접속 세션에 전송하는 경로입니다.

```text
이동 요청 수신
  → 순번 검사
  → 입력 방향·시간 간격 처리
  → 이동 후보 위치 계산
  → 지형·경계·장애물 기반 검증
  → 서버의 플레이어 상태 갱신
  → 처리 결과와 마지막 처리 순번 전송
```

### 설명을 작성한 코드 근거

분석 기준은 본인 저장소 `mjcjc/vida_de_granjero`의 `0f22a28` 버전입니다. 아래 내용은 실행으로 검증한 성과가 아니라 코드에서 확인한 처리입니다.

| 서버 처리 | 확인한 사실 | 코드 근거 |
|---|---|---|
| 요청 순번 검사 | 순번이 0이거나 마지막 처리 순번 이하이면 무시 | [ClientPacketHandler.cpp 164행](https://github.com/mjcjc/vida_de_granjero/blob/0f22a28/GameServer/ClientPacketHandler.cpp#L164-L172) |
| 방향 입력 처리 | 유한값을 검사하고 길이가 1보다 큰 입력을 정규화 | [ClientPacketHandler.cpp 180행](https://github.com/mjcjc/vida_de_granjero/blob/0f22a28/GameServer/ClientPacketHandler.cpp#L180-L195) |
| 이동 후보 계산 | 클라이언트·서버 시간 간격을 제한하고 현재 위치와 직전 저장 입력으로 수평 위치 계산 | [ClientPacketHandler.cpp 196행](https://github.com/mjcjc/vida_de_granjero/blob/0f22a28/GameServer/ClientPacketHandler.cpp#L196-L202) |
| 충돌 조건 수집 | 월드 장애물과 다른 플레이어의 영역을 모아 `ValidateMove` 호출 | [ClientPacketHandler.cpp 203행](https://github.com/mjcjc/vida_de_granjero/blob/0f22a28/GameServer/ClientPacketHandler.cpp#L203-L221) |
| 서버 상태 갱신 | 검증 결과 위치와 처리 순번·현재 입력 저장 | [ClientPacketHandler.cpp 237행](https://github.com/mjcjc/vida_de_granjero/blob/0f22a28/GameServer/ClientPacketHandler.cpp#L237-L244) |
| 이동 결과 공유 | `S_MOVE`에 마지막 처리 순번·서버 tick 등을 넣어 Broadcast | [ClientPacketHandler.cpp 246행](https://github.com/mjcjc/vida_de_granjero/blob/0f22a28/GameServer/ClientPacketHandler.cpp#L246-L258) |

검증 기준의 구현은 [ServerNavigation.cpp](https://github.com/mjcjc/vida_de_granjero/blob/0f22a28/GameServer/ServerNavigation.cpp)에 있습니다. 이 경로를 근거로 서버 이동 처리를 설명할 수 있지만, 부정행위 방지 완성이나 이동 오차 감소를 검증한 것으로 표현하지 않습니다.

### 이 주제를 후보로 고른 이유

서버 기술 이름을 나열하는 대신, 플레이어의 행동 하나가 어떤 검사와 상태 변경을 거쳐 다른 접속자에게 전달되는지 설명할 수 있습니다. 다만 이동 처리 중 어느 부분을 본인이 수정했는지 확인한 뒤 대표 경험으로 확정해야 합니다. 클라이언트에서 움직임을 부드럽게 표시하는 보간은 이 작업과 구분합니다.

## 보조 주제 후보

| 후보 | 서버에서 확인한 구현 | 근거 |
|---|---|---|
| 입장 시 월드 상태 전달 | 오브젝트 스냅샷과 현재 환경 상태 전송 | [입장 처리](https://github.com/mjcjc/vida_de_granjero/blob/0f22a28/GameServer/ClientPacketHandler.cpp#L72), [스냅샷](https://github.com/mjcjc/vida_de_granjero/blob/0f22a28/GameServer/GlobalWorld.cpp#L152) |
| 계정과 접속 세션 연결 | DB 인증 후 동일 계정의 접속 여부 검사, 세션 종료 시 연결 해제 | [AccountDB.cpp](https://github.com/mjcjc/vida_de_granjero/blob/0f22a28/GameServer/AccountDB.cpp), [GameSessionManager.cpp](https://github.com/mjcjc/vida_de_granjero/blob/0f22a28/GameServer/GameSessionManager.cpp) |

위 기능도 개인 담당으로 확정하지 않았습니다. 계정 구현은 비밀번호 평문 저장·비교가 있으므로 보안이 완성된 인증 시스템으로 소개하지 않습니다.

## 초안 확정에 필요한 확인

- 위 서버 코드 중 직접 작업하거나 GPT와 함께 수정한 부분은 어디인가?
- 당시 해결하려던 현상과 선택 이유를 기억하거나 기록으로 확인할 수 있는가?
- 어떤 실행으로 결과를 확인했으며, 해당 화면·로그가 남아 있는가?

담당 범위가 확인되면 그 부분만 본인 경험 문장으로 바꿉니다. 현재는 코드 분석 근거만 확보한 상태이며, 실행 검증이나 개인 기여를 확정하지 않았습니다.

이 문서는 서버 코드 분석을 바탕으로 한 검토 초안입니다. 개인 담당 범위와 실행 증거는 확인 후 보완합니다.
