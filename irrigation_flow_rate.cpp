/*
 * ============================================================
 *  Irrigation Flow Rate Calculator
 *  Language : C++17
 *  Purpose  : Calculate irrigation flow rates and water volumes
 *             for agricultural fields using standard formulas.
 * ============================================================
 */

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <cmath>
#include <limits>
#include <map>

// ── ANSI colour helpers ──────────────────────────────────────
#define RESET   "\033[0m"
#define BOLD    "\033[1m"
#define GREEN   "\033[32m"
#define CYAN    "\033[36m"
#define YELLOW  "\033[33m"
#define MAGENTA "\033[35m"

// ── Constants ───────────────────────────────────────────────
constexpr double MM_PER_M     = 1000.0;
constexpr double HA_TO_M2     = 10000.0;   // 1 ha = 10,000 m²
constexpr double MINS_PER_HR  = 60.0;
constexpr double SECS_PER_MIN = 60.0;

// ── Data structures ─────────────────────────────────────────
struct IrrigationResult {
    double flow_rate_lps;       // litres per second
    double flow_rate_m3ph;      // cubic metres per hour
    double total_volume_m3;     // total water required (m³)
    double application_time_hr; // hours needed to apply water
    double peak_demand_lps;     // peak demand with efficiency loss
};

// ── Utility: safe numeric input ─────────────────────────────
double getPositiveDouble(const std::string& prompt, double min_val = 0.001) {
    double value;
    while (true) {
        std::cout << CYAN << "  " << prompt << RESET << " ";
        if (std::cin >> value && value >= min_val) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << YELLOW << "  ⚠  Please enter a valid number > " << min_val << "\n" << RESET;
    }
}

int getIntChoice(const std::string& prompt, int min_v, int max_v) {
    int value;
    while (true) {
        std::cout << CYAN << "  " << prompt << RESET << " ";
        if (std::cin >> value && value >= min_v && value <= max_v) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << YELLOW << "  ⚠  Enter a number between " << min_v << " and " << max_v << "\n" << RESET;
    }
}

// ── Print a horizontal divider ───────────────────────────────
void divider(char ch = '─', int len = 56) {
    std::cout << std::string(len, ch) << "\n";
}

// ── FORMULA 1: Manning's formula for open-channel flow ──────
//   Q = (1/n) × A × R^(2/3) × S^(1/2)
//   Q  = flow (m³/s), n  = Manning's roughness coefficient
//   A  = cross-sectional area (m²)
//   R  = hydraulic radius = A / wetted-perimeter (m)
//   S  = channel slope (m/m)
double manningsFlow(double n, double width_m, double depth_m, double slope) {
    double area      = width_m * depth_m;          // rectangular channel
    double perimeter = width_m + 2.0 * depth_m;
    double R         = area / perimeter;
    double Q_m3s     = (1.0 / n) * area * std::pow(R, 2.0 / 3.0) * std::sqrt(slope);
    return Q_m3s * 1000.0;                         // convert to litres/second
}

// ── FORMULA 2: Drip / pressurised emitter flow ───────────────
//   q = Kd × H^x
//   q  = emitter discharge (L/h), Kd = emitter coefficient
//   H  = operating pressure (kPa), x  = emitter exponent
double emitterFlow_Lph(double Kd, double pressure_kPa, double x) {
    return Kd * std::pow(pressure_kPa, x);
}

// ── FORMULA 3: Field water requirement ───────────────────────
//   V = (ETc - Pe) × A × (1 / Ea)
//   ETc = crop evapotranspiration (mm/day)
//   Pe  = effective precipitation (mm/day)
//   A   = field area (ha)
//   Ea  = application efficiency (0–1)
double fieldWaterVolume_m3(double ETc_mm, double Pe_mm, double area_ha, double Ea) {
    double net_mm = std::max(0.0, ETc_mm - Pe_mm);
    double volume = (net_mm / MM_PER_M) * (area_ha * HA_TO_M2) / Ea;  // m³
    return volume;
}

// ── FORMULA 4: Flow rate from volume + time ──────────────────
double flowFromVolume_Lps(double volume_m3, double hours) {
    double secs = hours * MINS_PER_HR * SECS_PER_MIN;
    return (volume_m3 * 1000.0) / secs;   // L/s
}

// ── DISPLAY results ──────────────────────────────────────────
void printResult(const IrrigationResult& r) {
    divider('═');
    std::cout << BOLD << GREEN << "  📊  CALCULATION RESULTS\n" << RESET;
    divider('─');
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "  Flow Rate         : " << BOLD << r.flow_rate_lps    << RESET << " L/s\n";
    std::cout << "  Flow Rate         : " << BOLD << r.flow_rate_m3ph   << RESET << " m³/hr\n";
    std::cout << "  Total Volume      : " << BOLD << r.total_volume_m3  << RESET << " m³\n";
    std::cout << "  Application Time  : " << BOLD << r.application_time_hr << RESET << " hrs\n";
    std::cout << "  Peak Demand       : " << BOLD << r.peak_demand_lps  << RESET << " L/s (with efficiency)\n";
    divider('═');
}

// ────────────────────────────────────────────────────────────
//  MODULE A – Open Channel (Manning's)
// ────────────────────────────────────────────────────────────
void runMannings() {
    std::cout << "\n" << MAGENTA << BOLD << "  ── Manning's Open-Channel Flow ──\n" << RESET;
    std::cout << "  Formula: Q = (1/n) × A × R^(2/3) × S^(1/2)\n\n";

    // Roughness table
    std::map<int, std::pair<std::string, double>> roughness = {
        {1, {"Concrete-lined canal",    0.013}},
        {2, {"Earth channel (clean)",   0.022}},
        {3, {"Earth with vegetation",   0.035}},
        {4, {"Gravel / riprap channel", 0.030}},
    };
    std::cout << "  Select channel lining:\n";
    for (auto& [k, v] : roughness)
        std::cout << "    [" << k << "] " << v.first << " (n = " << v.second << ")\n";

    int choice  = getIntChoice("Choice [1-4]:", 1, 4);
    double n    = roughness[choice].second;
    double W    = getPositiveDouble("Channel width (m):");
    double D    = getPositiveDouble("Water depth (m):");
    double S    = getPositiveDouble("Bed slope (e.g. 0.001):", 0.0001);

    double Q_Lps   = manningsFlow(n, W, D, S);
    double Q_m3ph  = Q_Lps * 3.6;

    double area_ha = getPositiveDouble("Field area to irrigate (ha):");
    double app_mm  = getPositiveDouble("Target application depth (mm):");
    double V_m3    = (app_mm / MM_PER_M) * area_ha * HA_TO_M2;
    double T_hr    = V_m3 / (Q_m3ph);

    IrrigationResult r;
    r.flow_rate_lps    = Q_Lps;
    r.flow_rate_m3ph   = Q_m3ph;
    r.total_volume_m3  = V_m3;
    r.application_time_hr = T_hr;
    r.peak_demand_lps  = Q_Lps * 1.25;   // 25% safety factor
    printResult(r);
}

// ────────────────────────────────────────────────────────────
//  MODULE B – Drip Irrigation
// ────────────────────────────────────────────────────────────
void runDrip() {
    std::cout << "\n" << MAGENTA << BOLD << "  ── Drip Irrigation Flow Rate ──\n" << RESET;
    std::cout << "  Formula: q = Kd × H^x  (emitter discharge)\n\n";

    double Kd      = getPositiveDouble("Emitter coefficient Kd (e.g. 0.5):");
    double H       = getPositiveDouble("Operating pressure (kPa, e.g. 100):");
    double x       = getPositiveDouble("Emitter exponent x (e.g. 0.5):");
    double spacing = getPositiveDouble("Emitter spacing (m):");
    double rows    = getPositiveDouble("Number of laterals / rows:");
    double rowLen  = getPositiveDouble("Row length (m):");

    double q_Lph       = emitterFlow_Lph(Kd, H, x);
    long   totalEmit   = static_cast<long>(rows * (rowLen / spacing));
    double totalFlow_Lph = q_Lph * totalEmit;
    double totalFlow_Lps = totalFlow_Lph / 3600.0;

    double ETc  = getPositiveDouble("Crop ETc (mm/day):");
    double Pe   = getPositiveDouble("Effective rainfall Pe (mm/day, 0 if none):", 0.0);
    double area = getPositiveDouble("Field area (ha):");
    double Ea   = getPositiveDouble("Application efficiency (0.85 to 0.95):", 0.5);

    double V   = fieldWaterVolume_m3(ETc, Pe, area, Ea);
    double T   = V / (totalFlow_Lph / 1000.0);   // hours

    IrrigationResult r;
    r.flow_rate_lps    = totalFlow_Lps;
    r.flow_rate_m3ph   = totalFlow_Lph / 1000.0;
    r.total_volume_m3  = V;
    r.application_time_hr = T;
    r.peak_demand_lps  = totalFlow_Lps / Ea;

    std::cout << "  " << YELLOW << "Total emitters: " << totalEmit << RESET << "\n";
    std::cout << "  " << YELLOW << "Per-emitter q : " << std::fixed << std::setprecision(2)
              << q_Lph << " L/hr" << RESET << "\n";
    printResult(r);
}

// ────────────────────────────────────────────────────────────
//  MODULE C – Sprinkler / Field Demand
// ────────────────────────────────────────────────────────────
void runFieldDemand() {
    std::cout << "\n" << MAGENTA << BOLD << "  ── Field Water Demand & Flow Rate ──\n" << RESET;
    std::cout << "  Formula: V = (ETc - Pe) × A / Ea,  Q = V / t\n\n";

    double ETc  = getPositiveDouble("Crop ETc (mm/day):");
    double Pe   = getPositiveDouble("Effective rainfall (mm/day, 0 if none):", 0.0);
    double area = getPositiveDouble("Field area (ha):");
    double Ea   = getPositiveDouble("Irrigation efficiency (0–1, e.g. 0.75):", 0.1);
    double hrs  = getPositiveDouble("Daily operating hours of irrigation:");

    double V      = fieldWaterVolume_m3(ETc, Pe, area, Ea);
    double Q_Lps  = flowFromVolume_Lps(V, hrs);
    double Q_m3ph = Q_Lps * 3.6;

    IrrigationResult r;
    r.flow_rate_lps    = Q_Lps;
    r.flow_rate_m3ph   = Q_m3ph;
    r.total_volume_m3  = V;
    r.application_time_hr = hrs;
    r.peak_demand_lps  = Q_Lps * 1.15;   // 15% peak factor
    printResult(r);
}

// ────────────────────────────────────────────────────────────
//  MAIN
// ────────────────────────────────────────────────────────────
int main() {
    std::cout << "\n";
    divider('═');
    std::cout << BOLD << GREEN
              << "       💧  IRRIGATION FLOW RATE CALCULATOR\n"
              << RESET;
    divider('═');
    std::cout << "  Calculates flow rates using hydraulic formulas\n"
              << "  for open-channel, drip, and sprinkler systems.\n";
    divider();

    bool running = true;
    while (running) {
        std::cout << "\n  " << BOLD << "Select Calculation Method:\n" << RESET;
        std::cout << "    [1]  Open-Channel Flow  (Manning's formula)\n";
        std::cout << "    [2]  Drip Irrigation    (Emitter discharge)\n";
        std::cout << "    [3]  Field Water Demand (ETc-based)\n";
        std::cout << "    [0]  Exit\n\n";

        int sel = getIntChoice("Your choice [0-3]:", 0, 3);
        switch (sel) {
            case 1: runMannings();    break;
            case 2: runDrip();        break;
            case 3: runFieldDemand(); break;
            case 0: running = false;  break;
        }
        if (running) {
            std::cout << "\n" << CYAN
                      << "  Press ENTER to continue..." << RESET;
            std::cin.get();
        }
    }

    std::cout << GREEN << BOLD
              << "\n  👋  Thank you for using the Irrigation Calculator!\n\n"
              << RESET;
    return 0;
}
