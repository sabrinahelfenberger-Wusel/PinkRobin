#pragma once

#include "pinkrobin/output.hpp"

namespace pinkrobin {

    /// Erzeugt den deutschen Anzeigetext für eine Ausgabe.
    /// Technische Wetterabrufe erhalten einen leeren Text.
    /// Bei einem Fehler bleibt text leer.
    bool make_output_text(
        const OutputPayload& payload,
        OutputText& text) noexcept;

}