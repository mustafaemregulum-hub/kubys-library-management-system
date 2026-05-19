#pragma once

#include "depo.h"
#include "varliklar.h"

#include <algorithm>
#include <iterator>
#include <map>
#include <optional>
#include <ranges>
#include <string>
#include <vector>

namespace kubys::sorgu {

inline std::vector<Kitap> kategoriye_gore_kitaplari_listele(
    const Depo<std::string, Kitap>& kitaplar,
    const std::string& kategori)
{
    std::vector<Kitap> sonuc;

    auto filtreliKitaplar = kitaplar.tumunu_al()
        | std::views::values
        | std::views::filter([&kategori](const Kitap& kitap) {
              return kitap.kategori == kategori;
          });

    std::ranges::copy(filtreliKitaplar, std::back_inserter(sonuc));
    return sonuc;
}

inline std::vector<OduncKaydi> gecikmis_iade_kayitlarini_bul(
    const Depo<int, OduncKaydi>& oduncler)
{
    std::vector<OduncKaydi> sonuc;

    auto gecikmisKayitlar = oduncler.tumunu_al()
        | std::views::values
        | std::views::filter([](const OduncKaydi& kayit) {
              return kayit.durum == OduncDurum::Gecikmis;
          });

    std::ranges::copy(gecikmisKayitlar, std::back_inserter(sonuc));
    return sonuc;
}

inline std::vector<Kitap> kitaplari_yayin_yilina_gore_sirala(
    const Depo<std::string, Kitap>& kitaplar)
{
    std::vector<Kitap> sonuc;
    std::ranges::copy(kitaplar.tumunu_al() | std::views::values, std::back_inserter(sonuc));

    std::ranges::sort(sonuc, {}, &Kitap::yayin_yili);
    return sonuc;
}

inline std::size_t uyenin_toplam_odunc_sayisini_hesapla(
    const Depo<int, OduncKaydi>& oduncler,
    int uyeNo)
{
    return static_cast<std::size_t>(std::ranges::count_if(
        oduncler.tumunu_al() | std::views::values,
        [uyeNo](const OduncKaydi& kayit) {
            return kayit.uye_no == uyeNo;
        }
    ));
}

inline std::optional<Kitap> en_cok_odunc_alinan_kitabi_bul(
    const Depo<std::string, Kitap>& kitaplar,
    const Depo<int, OduncKaydi>& oduncler)
{
    std::map<std::string, int> oduncSayilari;

    for (const auto& kayit : oduncler.tumunu_al() | std::views::values) {
        ++oduncSayilari[kayit.isbn];
    }

    if (oduncSayilari.empty()) {
        return std::nullopt;
    }

    const auto enCok = std::ranges::max_element(
        oduncSayilari,
        [](const auto& sol, const auto& sag) {
            return sol.second < sag.second;
        }
    );

    if (enCok == oduncSayilari.end()) {
        return std::nullopt;
    }

    return kitaplar.bul(enCok->first);
}

} // namespace kubys::sorgu
