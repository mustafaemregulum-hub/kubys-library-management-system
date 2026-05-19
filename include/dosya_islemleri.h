#pragma once

#include "depo.h"
#include "varliklar.h"

#include <cstdint>
#include <fstream>
#include <limits>
#include <optional>
#include <string>
#include <type_traits>

namespace kubys::dosya {

struct VeriPaketi {
    Depo<std::string, Kitap> kitaplar;
    Depo<int, Uye> uyeler;
    Depo<int, OduncKaydi> oduncler;
};

namespace detay {

inline constexpr std::uint64_t MAKSIMUM_KAYIT_SAYISI = 1'000'000;
inline constexpr std::uint64_t MAKSIMUM_METIN_UZUNLUGU = 16'384;

inline bool yaz_uint64(std::ostream& cikis, std::uint64_t deger) {
    cikis.write(reinterpret_cast<const char*>(&deger), sizeof(deger));
    return static_cast<bool>(cikis);
}

inline bool oku_uint64(std::istream& giris, std::uint64_t& deger) {
    giris.read(reinterpret_cast<char*>(&deger), sizeof(deger));
    return static_cast<bool>(giris);
}

inline bool yaz_int(std::ostream& cikis, int deger) {
    cikis.write(reinterpret_cast<const char*>(&deger), sizeof(deger));
    return static_cast<bool>(cikis);
}

inline bool oku_int(std::istream& giris, int& deger) {
    giris.read(reinterpret_cast<char*>(&deger), sizeof(deger));
    return static_cast<bool>(giris);
}

inline bool yaz_bool(std::ostream& cikis, bool deger) {
    cikis.write(reinterpret_cast<const char*>(&deger), sizeof(deger));
    return static_cast<bool>(cikis);
}

inline bool oku_bool(std::istream& giris, bool& deger) {
    giris.read(reinterpret_cast<char*>(&deger), sizeof(deger));
    return static_cast<bool>(giris);
}

inline bool yaz_string(std::ostream& cikis, const std::string& metin) {
    if (metin.size() > MAKSIMUM_METIN_UZUNLUGU) {
        return false;
    }

    if (!yaz_uint64(cikis, static_cast<std::uint64_t>(metin.size()))) {
        return false;
    }

    cikis.write(metin.data(), static_cast<std::streamsize>(metin.size()));
    return static_cast<bool>(cikis);
}

inline bool oku_string(std::istream& giris, std::string& metin) {
    std::uint64_t uzunluk = 0;
    if (!oku_uint64(giris, uzunluk) || uzunluk > MAKSIMUM_METIN_UZUNLUGU) {
        return false;
    }

    metin.assign(static_cast<std::size_t>(uzunluk), '\0');

    if (uzunluk == 0) {
        return true;
    }

    giris.read(metin.data(), static_cast<std::streamsize>(uzunluk));
    return static_cast<bool>(giris);
}

inline bool yaz_optional_string(std::ostream& cikis, const std::optional<std::string>& metin) {
    if (!yaz_bool(cikis, metin.has_value())) {
        return false;
    }

    if (!metin.has_value()) {
        return true;
    }

    return yaz_string(cikis, metin.value());
}

inline bool oku_optional_string(std::istream& giris, std::optional<std::string>& metin) {
    bool varMi = false;
    if (!oku_bool(giris, varMi)) {
        return false;
    }

    if (!varMi) {
        metin = std::nullopt;
        return true;
    }

    std::string okunan;
    if (!oku_string(giris, okunan)) {
        return false;
    }

    metin = okunan;
    return true;
}

inline bool yaz_kitap(std::ostream& cikis, const Kitap& kitap) {
    return yaz_string(cikis, kitap.isbn)
        && yaz_string(cikis, kitap.baslik)
        && yaz_string(cikis, kitap.yazar)
        && yaz_int(cikis, kitap.yayin_yili)
        && yaz_string(cikis, kitap.kategori)
        && yaz_int(cikis, kitap.kopya_sayisi);
}

inline bool oku_kitap(std::istream& giris, Kitap& kitap) {
    return oku_string(giris, kitap.isbn)
        && oku_string(giris, kitap.baslik)
        && oku_string(giris, kitap.yazar)
        && oku_int(giris, kitap.yayin_yili)
        && oku_string(giris, kitap.kategori)
        && oku_int(giris, kitap.kopya_sayisi);
}

inline bool yaz_uye(std::ostream& cikis, const Uye& uye) {
    return yaz_int(cikis, uye.uye_no)
        && yaz_string(cikis, uye.isim)
        && yaz_string(cikis, uye.soyisim)
        && yaz_string(cikis, uye.telefon)
        && yaz_string(cikis, uye.kayit_tarihi);
}

inline bool oku_uye(std::istream& giris, Uye& uye) {
    return oku_int(giris, uye.uye_no)
        && oku_string(giris, uye.isim)
        && oku_string(giris, uye.soyisim)
        && oku_string(giris, uye.telefon)
        && oku_string(giris, uye.kayit_tarihi);
}

inline bool yaz_odunc(std::ostream& cikis, const OduncKaydi& kayit) {
    const int durumDegeri = static_cast<int>(kayit.durum);

    return yaz_int(cikis, kayit.kayit_id)
        && yaz_int(cikis, kayit.uye_no)
        && yaz_string(cikis, kayit.isbn)
        && yaz_string(cikis, kayit.odunc_tarihi)
        && yaz_optional_string(cikis, kayit.iade_tarihi)
        && yaz_int(cikis, durumDegeri);
}

inline bool oku_odunc(std::istream& giris, OduncKaydi& kayit) {
    int durumDegeri = 0;

    if (!(oku_int(giris, kayit.kayit_id)
        && oku_int(giris, kayit.uye_no)
        && oku_string(giris, kayit.isbn)
        && oku_string(giris, kayit.odunc_tarihi)
        && oku_optional_string(giris, kayit.iade_tarihi)
        && oku_int(giris, durumDegeri))) {
        return false;
    }

    if (durumDegeri < static_cast<int>(OduncDurum::Oduncte)
        || durumDegeri > static_cast<int>(OduncDurum::Gecikmis)) {
        return false;
    }

    kayit.durum = static_cast<OduncDurum>(durumDegeri);
    return true;
}

inline bool yaz_kitap_deposu(std::ostream& cikis, const Depo<std::string, Kitap>& depo) {
    if (!yaz_uint64(cikis, static_cast<std::uint64_t>(depo.boyut()))) {
        return false;
    }

    for (const auto& [anahtar, kitap] : depo.tumunu_al()) {
        if (!yaz_string(cikis, anahtar) || !yaz_kitap(cikis, kitap)) {
            return false;
        }
    }

    return true;
}

inline bool oku_kitap_deposu(std::istream& giris, Depo<std::string, Kitap>& depo) {
    std::uint64_t adet = 0;
    if (!oku_uint64(giris, adet) || adet > MAKSIMUM_KAYIT_SAYISI) {
        return false;
    }

    depo.temizle();

    for (std::uint64_t i = 0; i < adet; ++i) {
        std::string anahtar;
        Kitap kitap;

        if (!oku_string(giris, anahtar) || !oku_kitap(giris, kitap)) {
            return false;
        }

        depo.ekle(anahtar, kitap);
    }

    return true;
}

inline bool yaz_uye_deposu(std::ostream& cikis, const Depo<int, Uye>& depo) {
    if (!yaz_uint64(cikis, static_cast<std::uint64_t>(depo.boyut()))) {
        return false;
    }

    for (const auto& [anahtar, uye] : depo.tumunu_al()) {
        if (!yaz_int(cikis, anahtar) || !yaz_uye(cikis, uye)) {
            return false;
        }
    }

    return true;
}

inline bool oku_uye_deposu(std::istream& giris, Depo<int, Uye>& depo) {
    std::uint64_t adet = 0;
    if (!oku_uint64(giris, adet) || adet > MAKSIMUM_KAYIT_SAYISI) {
        return false;
    }

    depo.temizle();

    for (std::uint64_t i = 0; i < adet; ++i) {
        int anahtar = 0;
        Uye uye;

        if (!oku_int(giris, anahtar) || !oku_uye(giris, uye)) {
            return false;
        }

        depo.ekle(anahtar, uye);
    }

    return true;
}

inline bool yaz_odunc_deposu(std::ostream& cikis, const Depo<int, OduncKaydi>& depo) {
    if (!yaz_uint64(cikis, static_cast<std::uint64_t>(depo.boyut()))) {
        return false;
    }

    for (const auto& [anahtar, kayit] : depo.tumunu_al()) {
        if (!yaz_int(cikis, anahtar) || !yaz_odunc(cikis, kayit)) {
            return false;
        }
    }

    return true;
}

inline bool oku_odunc_deposu(std::istream& giris, Depo<int, OduncKaydi>& depo) {
    std::uint64_t adet = 0;
    if (!oku_uint64(giris, adet) || adet > MAKSIMUM_KAYIT_SAYISI) {
        return false;
    }

    depo.temizle();

    for (std::uint64_t i = 0; i < adet; ++i) {
        int anahtar = 0;
        OduncKaydi kayit;

        if (!oku_int(giris, anahtar) || !oku_odunc(giris, kayit)) {
            return false;
        }

        depo.ekle(anahtar, kayit);
    }

    return true;
}

} // namespace detay

template <typename Anahtar, typename Deger>
bool dosyaya_kaydet(const std::string& dosyaYolu, const Depo<Anahtar, Deger>& depo) {
    std::ofstream cikis(dosyaYolu, std::ios::binary);

    if (!cikis) {
        return false;
    }

    if constexpr (std::is_same_v<Anahtar, std::string> && std::is_same_v<Deger, Kitap>) {
        return detay::yaz_string(cikis, "KUBYS_KITAP_DEPOSU_1")
            && detay::yaz_kitap_deposu(cikis, depo);
    } else if constexpr (std::is_same_v<Anahtar, int> && std::is_same_v<Deger, Uye>) {
        return detay::yaz_string(cikis, "KUBYS_UYE_DEPOSU_1")
            && detay::yaz_uye_deposu(cikis, depo);
    } else if constexpr (std::is_same_v<Anahtar, int> && std::is_same_v<Deger, OduncKaydi>) {
        return detay::yaz_string(cikis, "KUBYS_ODUNC_DEPOSU_1")
            && detay::yaz_odunc_deposu(cikis, depo);
    } else {
        static_assert(sizeof(Deger) == 0, "Bu depo tipi icin binary kayit destegi tanimli degil.");
    }
}

template <typename Anahtar, typename Deger>
std::optional<Depo<Anahtar, Deger>> dosyadan_oku(const std::string& dosyaYolu) {
    std::ifstream giris(dosyaYolu, std::ios::binary);

    if (!giris) {
        return std::nullopt;
    }

    std::string imza;
    if (!detay::oku_string(giris, imza)) {
        return std::nullopt;
    }

    Depo<Anahtar, Deger> depo;

    if constexpr (std::is_same_v<Anahtar, std::string> && std::is_same_v<Deger, Kitap>) {
        if (imza != "KUBYS_KITAP_DEPOSU_1" || !detay::oku_kitap_deposu(giris, depo)) {
            return std::nullopt;
        }
    } else if constexpr (std::is_same_v<Anahtar, int> && std::is_same_v<Deger, Uye>) {
        if (imza != "KUBYS_UYE_DEPOSU_1" || !detay::oku_uye_deposu(giris, depo)) {
            return std::nullopt;
        }
    } else if constexpr (std::is_same_v<Anahtar, int> && std::is_same_v<Deger, OduncKaydi>) {
        if (imza != "KUBYS_ODUNC_DEPOSU_1" || !detay::oku_odunc_deposu(giris, depo)) {
            return std::nullopt;
        }
    } else {
        static_assert(sizeof(Deger) == 0, "Bu depo tipi icin binary okuma destegi tanimli degil.");
    }

    return depo;
}

inline bool kubys_verilerini_dosyaya_kaydet(
    const std::string& dosyaYolu,
    const Depo<std::string, Kitap>& kitaplar,
    const Depo<int, Uye>& uyeler,
    const Depo<int, OduncKaydi>& oduncler)
{
    std::ofstream cikis(dosyaYolu, std::ios::binary);

    if (!cikis) {
        return false;
    }

    return detay::yaz_string(cikis, "KUBYS_TUM_VERI_1")
        && detay::yaz_kitap_deposu(cikis, kitaplar)
        && detay::yaz_uye_deposu(cikis, uyeler)
        && detay::yaz_odunc_deposu(cikis, oduncler);
}

inline std::optional<VeriPaketi> kubys_verilerini_dosyadan_oku(const std::string& dosyaYolu) {
    std::ifstream giris(dosyaYolu, std::ios::binary);

    if (!giris) {
        return std::nullopt;
    }

    std::string imza;
    if (!detay::oku_string(giris, imza) || imza != "KUBYS_TUM_VERI_1") {
        return std::nullopt;
    }

    VeriPaketi paket;

    if (!detay::oku_kitap_deposu(giris, paket.kitaplar)
        || !detay::oku_uye_deposu(giris, paket.uyeler)
        || !detay::oku_odunc_deposu(giris, paket.oduncler)) {
        return std::nullopt;
    }

    return paket;
}

} // namespace kubys::dosya
