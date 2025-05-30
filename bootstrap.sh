#!/bin/bash
set -e  # Bricht bei Fehler ab

echo "🔧 Klonen von esp-matter..."
git clone https://github.com/espressif/esp-matter.git
cd esp-matter

echo "📦 Patch anwenden..."
git apply ../esp_matter_modifications.patch

echo "✅ Fertig! esp-matter ist vorbereitet."
