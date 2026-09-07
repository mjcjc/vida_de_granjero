#include "ClientNetwork.h"


bool ClientNetwork::Start(const wchar_t* ip, uint16 port)
{
    ServerPacketHandler::Init();
    std::this_thread::sleep_for(std::chrono::seconds(1));

    _service = MakeShared<ClientService>(
        NetAddress(ip, port),
        MakeShared<IocpCore>(),
        [this]() -> SessionRef
        {
            auto s = MakeShared<ClientSession>();
            _session = s;                 
            return s;
        },
        1);

    if (_service->Start() == false)   // 여기서 실제 연결 시도
    {
        std::wcout << L"[ClientNetwork] ClientService::Start() 실패\n";
        _service.reset();
        _connected = false;
        return false;
    }

    _connected = true;

    // IOCP 디스패치 스레드
    GThreadManager->Launch([service = _service]()
        {
            while (true)
            {
                service->GetIocpCore()->Dispatch();
            }
        });

    return true;
}

void ClientNetwork::Broadcast(SendBufferRef sendBuffer)
{
    auto service = _service;     // refcount 증가로 수명 고정
    if (!service) return;
    service->Broadcast(sendBuffer);
}
void ClientNetwork::SendToServer(SendBufferRef sendBuffer)
{
    if (!_session)
    {
        cout << "[SendToServer] session is null (not connected?)" << endl;
        return;
    }

    _session->Send(sendBuffer);
    //cout << "[SendToServer] sent to server" << endl;
}
