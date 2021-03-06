/******************************************************************************
 * Copyright (c) 2019-2020, NVIDIA CORPORATION. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of NVIDIA CORPORATION nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

// examples/mdl_sdk/dxr/mdl_d3d12/utils.h

#ifndef MDL_D3D12_UTILS_H
#define MDL_D3D12_UTILS_H

#include <string>
#include <vector>
#include <chrono>
#include <cmath>
#include <Windows.h>
#include <DirectXMath.h>

#define SRC __FILE__,__LINE__

struct ID3D12Object;

namespace mdl_d3d12
{
    const float PI = 3.14159265358979323846f;
    const float PI_OVER_2 = PI * 0.5f;
    const float ONE_OVER_PI = 0.318309886183790671538f;

    void log_info(
        const std::string& message,
        const std::string& file = "",
        int line = 0);

    void log_warning(
        const std::string& message,
        const std::string& file = "",
        int line = 0);

    void log_error(
        const std::string& message, 
        const std::string& file = "", 
        int line = 0);

    void log_error(
        const std::exception& exception,
        const std::string& file,
        int line = 0);

    bool log_on_failure(
        HRESULT error_code, 
        const std::string& message, 
        const std::string& file = "", 
        int line = 0);

    void throw_on_failure(
        HRESULT error_code,
        const std::string& message,
        const std::string& file = "",
        int line = 0);

    void log_set_file_path(const char* log_file_path);

    template<typename T>
    using is_scoped_enum = 
        std::integral_constant<bool, std::is_enum<T>::value && !std::is_convertible<T, int>::value>;

    template<class T> constexpr T operator & (T a, T b) { 
        return static_cast<T>(static_cast<int>(a) & static_cast<int>(b)); 
    }

    template<class T> constexpr T operator | (T a, T b) { 
        return static_cast<T>(static_cast<int>(a) | static_cast<int>(b)); 
    }

    template<class T, typename = typename std::enable_if<is_scoped_enum<T>::value>::type>
    inline bool has_flag(T a, T toCheck) { return static_cast<int>(a & toCheck) > 0; }

    void set_debug_name(ID3D12Object* obj, const std::string& name);

    inline std::wstring str_to_wstr(const std::string& s)
    {
        size_t slength = s.length() + 1;
        int len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), static_cast<int>(slength), nullptr, 0);
        wchar_t* buf = new wchar_t[len];
        MultiByteToWideChar(CP_ACP, 0, s.c_str(), static_cast<int>(slength), buf, len);
        std::wstring r(buf);
        delete[] buf;
        return r;
    }

    inline std::string wstr_to_str(const std::wstring& s)
    {
        return std::string(s.begin(), s.end());
    }

    inline size_t round_to_power_of_two(size_t value, size_t power_of_two_factor)
    {
        return (value + (power_of_two_factor - 1)) & ~(power_of_two_factor - 1);
    }

    inline std::vector<std::string> str_split(const std::string& input, char sep)
    {
        std::vector<std::string> chunks;

        size_t offset(0);
        size_t pos(0);
        std::string chunk;
        while (pos != std::string::npos)
        {
            pos = input.find(sep, offset);

            if (pos == std::string::npos)
                chunk = input.substr(offset);
            else
                chunk = input.substr(offset, pos - offset);

            if(!chunk.empty())
                chunks.push_back(chunk);
            offset = pos + 1;
        }
        return chunks;
    }


    inline bool str_starts_with(const std::string& s, const std::string& potential_start)
    {
        size_t n = potential_start.size();

        if (s.size() < n) 
            return false;

        for (size_t i = 0; i < n; ++i)
            if (s[i] != potential_start[i])
                return false;

        return true;
    }

    inline bool str_ends_with(const std::string& s, const std::string& potential_end)
    {
        size_t n = potential_end.size();
        size_t sn = s.size();

        if (sn < n)
            return false;

        for (size_t i = 0; i < n; ++i)
            if (s[sn - i - 1] != potential_end[n - i - 1])
                return false;

        return true;
    }

    inline bool str_remove_quotes(std::string& s)
    {
        size_t l = s.length();
        if (l < 2) 
            return false;

        bool leading = s[0] == '\"';
        bool trailing = s[l-1] == '\"';

        if (leading != trailing)
            return false;

        if(leading)
            s = s.substr(1, l - 2);
        return true;
    }

    struct Timing
    {
        explicit Timing(std::string operation);
        virtual ~Timing();

    private:
        std::string m_operation;
        std::chrono::steady_clock::time_point m_start;
    };

    // --------------------------------------------------------------------------------------------
    // Vector Math 
    // --------------------------------------------------------------------------------------------

    inline DirectX::XMMATRIX inverse(const DirectX::XMMATRIX& m, DirectX::XMVECTOR* determinants = nullptr)
    {
        if (determinants)
            return DirectX::XMMatrixInverse(determinants, m);

        DirectX::XMVECTOR det;
        return DirectX::XMMatrixInverse(&det, m);
    }


    // --------------------------------------------------------------------------------------------
    // Float Math 
    // --------------------------------------------------------------------------------------------

    inline DirectX::XMFLOAT3 normalize(const DirectX::XMFLOAT3& v)
    {
        float inv_length = 1.0f / std::sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
        return {v.x * inv_length, v.y * inv_length, v.z * inv_length};
    }

    inline float length(const DirectX::XMFLOAT3& v)
    {
        return std::sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    }

    inline float length2(const DirectX::XMFLOAT3& v)
    {
        return v.x * v.x + v.y * v.y + v.z * v.z;
    }

    inline float dot(const DirectX::XMFLOAT3& v1, const DirectX::XMFLOAT3& v2)
    {
        return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
    }


    inline DirectX::XMFLOAT3 cross(const DirectX::XMFLOAT3& v1, const DirectX::XMFLOAT3& v2)
    {
        DirectX::XMFLOAT3 res;
        res.x = (v1.y * v2.z) - (v1.z * v2.y);
        res.y = (v1.z * v2.x) - (v1.x * v2.z);
        res.z = (v1.x * v2.y) - (v1.y * v2.x);
        return res;
    }

    inline DirectX::XMFLOAT3 operator-(const DirectX::XMFLOAT3& v1, const DirectX::XMFLOAT3& v2)
    {
        DirectX::XMFLOAT3 res;
        res.x = v1.x - v2.x;
        res.y = v1.y - v2.y;
        res.z = v1.z - v2.z;
        return res;
    }

    inline DirectX::XMFLOAT3 operator*(const DirectX::XMFLOAT3& v1, float s)
    {
        DirectX::XMFLOAT3 res;
        res.x = v1.x * s;
        res.y = v1.y * s;
        res.z = v1.z * s;
        return res;
    }


    inline void operator*=(DirectX::XMFLOAT3& v, float s)
    {
        v.x *= s;
        v.y *= s;
        v.z *= s;
    }

    inline DirectX::XMFLOAT3 operator-(const DirectX::XMFLOAT3& v1)
    {
        DirectX::XMFLOAT3 res;
        res.x = v1.x;
        res.y = -v1.y;
        res.z = -v1.z;
        return res;
    }

    inline void operator+=(DirectX::XMFLOAT3& v1, const DirectX::XMFLOAT3& v2)
    {
        v1.x += v2.x;
        v1.y += v2.y;
        v1.z += v2.z;
    }
}

#endif
