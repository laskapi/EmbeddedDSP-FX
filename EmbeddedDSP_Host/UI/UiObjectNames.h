#ifndef EMBEDDEDDSP_HOST_UIOBJECTNAMES_H
#define EMBEDDEDDSP_HOST_UIOBJECTNAMES_H

namespace Host::UI::UiObjectNames {

/// @brief QWidget objectName values; must match #id selectors in UI/style.qss.

inline constexpr char kSpectrumWelcomeLabel[] = "SpectrumWelcomeLabel";
inline constexpr char kSpectrumZoomIcon[] = "SpectrumZoomIcon";
inline constexpr char kSpectrumSensitivitySlider[] = "SpectrumSensitivitySlider";
inline constexpr char kSpectrumUnitLabel[] = "SpectrumUnitLabel";
inline constexpr char kSpectrumAxisX[] = "SpectrumAxisX";

inline constexpr char kEffectItemBox[] = "effectItemBox";
inline constexpr char kEffectChainArrow[] = "effectChainArrow";

inline constexpr char kMasterVolumeBox[] = "masterVolumeBox";
inline constexpr char kMasterVolumeTitle[] = "masterVolumeTitle";
inline constexpr char kMasterVolumeSlider[] = "masterVolumeSlider";
inline constexpr char kMasterVolumeValue[] = "masterVolumeValue";

} // namespace Host::UI::UiObjectNames

#endif // EMBEDDEDDSP_HOST_UIOBJECTNAMES_H
