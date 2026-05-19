#pragma once

/* depo.h -- Genel Amacli Depo Sablon Sinifi
 Anahtar-deger cifti olarak veri saklayan sablon sinif.*/

#include <algorithm>
#include <functional>
#include <map>
#include <optional>
#include <ranges>
#include <vector>

template <typename Anahtar, typename Deger>
class Depo {
public:
    // Yeni kayit ekler. Anahtar zaten varsa false doner.
    bool ekle(const Anahtar& anahtar, const Deger& deger) {
        auto [it, eklendi] = m_veriler.insert({anahtar, deger});
        return eklendi;
    }

    // Verilen anahtari map'te arar.
    // Bulursa std::optional<Deger>, bulamazsa std::nullopt doner.
    std::optional<Deger> bul(const Anahtar& anahtar) const {
        auto it = m_veriler.find(anahtar);
        if (it != m_veriler.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    // Verilen anahtari map'ten siler.
    // Basariliysa true, bulunamazsa false doner.
    bool sil(const Anahtar& anahtar) {
        return m_veriler.erase(anahtar) > 0;
    }

    // Var olan kaydi gunceller. Anahtar yoksa false doner.
    bool guncelle(const Anahtar& anahtar, const Deger& deger) {
        auto it = m_veriler.find(anahtar);
        if (it == m_veriler.end()) {
            return false;
        }

        it->second = deger;
        return true;
    }

    // Anahtar depoda var mi kontrol eder.
    bool var_mi(const Anahtar& anahtar) const {
        return m_veriler.contains(anahtar);
    }

    // Tum kayitlari doner (salt okunur)
    const std::map<Anahtar, Deger>& tumunu_al() const {
        return m_veriler;
    }

    // Kayit sayisini doner
    std::size_t boyut() const {
        return m_veriler.size();
    }

    // Depoyu temizler
    void temizle() {
        m_veriler.clear();
    }

    // Kosulu saglayan tum degerleri vector olarak doner.
    std::vector<Deger> filtrele(std::function<bool(const Deger&)> kosul) const {
        std::vector<Deger> sonuc;

        for (const auto& deger : m_veriler | std::views::values) {
            if (kosul(deger)) {
                sonuc.push_back(deger);
            }
        }

        return sonuc;
    }

private:
    std::map<Anahtar, Deger> m_veriler;
};