#include "Common.hlsli"
#include "TileableNoise.hlsli"

struct VS_INPUT
{
    float3 position : POSITION0;
    float3 particleState : POSITION1; // x: lifeTime, y: speed, z: spawnInterval
    float2 size : TEXCOORD;
};

struct VS_OUTPUT
{
    float3 positionW : POSITION;
    float2 size : TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;

    // =========================================================
    // 1. 파티클 기본 파라미터
    // =========================================================
    float lifeTime = input.particleState.x;
    float speed = input.particleState.y;
    float spawnInterval = input.particleState.z;
    float startTime = fStartTime;

    float seed = dot(input.particleState, float3(12.9898, 78.233, 37.719));
    float rand01 = frac(sin(seed) * 43758.5453);

    float t = fmod(fTime + rand01 * spawnInterval, lifeTime);
    float normalizedTime = saturate(t / lifeTime);

    // =========================================================
    // 2. 중심 위치 (GameObject World)
    // =========================================================
    float3 centerW = gmtxWorldCb._41_42_43;
    float3 posW = centerW;

    // =========================================================
    // 3. Spawn 고정 시드
    // =========================================================
    float spawnIndex = floor((fTime + rand01 * spawnInterval) / spawnInterval);
    float spawnSeed = seed + spawnIndex * 17.0;
    float3 h = hash33(float3(spawnSeed, spawnSeed * 2.0, spawnSeed * 3.0));

    // =========================================================
    // 4. 타입별 Spawn + 이동
    // =========================================================

    // ---------------- Rain / Snow ----------------
    if (nParticleFlags & (PF_RAIN | PF_SNOW))
    {
        float2 dirXZ = normalize(h.xz);
        float radius = sqrt(frac(h.y)) * fSpawnRadius;
        float spawnHeight = 7.0;

        posW = centerW + float3(
            dirXZ.x * radius,
            spawnHeight,
            dirXZ.y * radius
        );

        if (nParticleFlags & PF_RAIN)
        {
            posW.y += -speed * t;
        }
        else // SNOW
        {
            float sway = sin(fTime + spawnSeed) * 0.5;
            posW += float3(sway, -speed * t * 0.3, cos(fTime + spawnSeed) * 0.3);
        }
    }

    // ---------------- Dust ----------------
    else if (nParticleFlags & PF_DUST)
    {
        float3 dir = normalize(h);
        float r = pow(frac(h.z), 1.0 / 3.0) * fSpawnRadius;

        posW = centerW + dir * r;

        float rise = speed * t * 0.1;

        float3 dustNoise = float3(
            perlinfbm(posW + fTime * 0.05, 2.0, 2),
            perlinfbm(posW + fTime * 0.03, 2.0, 2),
            perlinfbm(posW + fTime * 0.04, 2.0, 2)
        );

        posW += dustNoise * 0.5;
        posW.y += rise;
    }

    // ---------------- Punch (폭죽 / 파편) ----------------
    else if (nParticleFlags & PF_PUNCH)
    {
        // 경과 시간 (CPU에서 누적해서 넘긴 값)
        float ttime = fStartTime;

        // 수명 초과 → 화면 밖으로
        if (ttime >= lifeTime)
        {
            posW = centerW + float3(0.0f, 1000.0f, 0.0f);
        }
        else
        {
            // 방향: 완전 랜덤 (폭죽)
            float3 dir = normalize(h * 2.0f - 1.0f);

            // 속도 스케일 (추천값)
            float punchSpeed = speed; // CPU에서 2~6 추천

            float3 velocity = dir * punchSpeed;

            // 중력 (과하지 않게)
            float gravity = -4.5f;

            //  정석 물리식
            posW = centerW
                + velocity * ttime
                + float3(0.0f, 0.5f * gravity * ttime * ttime, 0.0f);
        }
    }

  
    // 5. 공통 노이즈 (선언 먼저!)
    float3 noiseOffset = float3(0, 0, 0);

    if (nParticleFlags & PF_USE_NOISE)
    {
        float noise = perlinfbm(
            posW * 0.2 + fTime * 0.1,
            4.0,
            3
        );

        noiseOffset = noise * float3(1.0, 0.3, 1.0);
    }

    // Punch에는 노이즈 약하게만
    if (nParticleFlags & PF_PUNCH)
        posW += noiseOffset * 0.1;
    else
        posW += noiseOffset;

    
    // 6. Block 충돌 밀어내기

    [loop]
        for (uint i = 0; i < blockCount; ++i)
        {
            WeatherBlock b = blocks[i];

            float3 delta = posW - b.center;
            float3 absD = abs(delta);

            if (absD.x < b.extent.x &&
                absD.y < b.extent.y &&
                absD.z < b.extent.z)
            {
                float3 push = b.extent - absD;

                if (push.x < push.y && push.x < push.z)
                    posW.x = b.center.x + sign(delta.x) * b.extent.x;
                else if (push.y < push.z)
                    posW.y = b.center.y + sign(delta.y) * b.extent.y;
                else
                    posW.z = b.center.z + sign(delta.z) * b.extent.z;
            }
        }

    output.positionW = posW;
    output.size = input.size;
    return output;
}