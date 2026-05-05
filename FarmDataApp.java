/*
 * ============================================================
 *  FarmDataApp.java
 *  A console-based Java application for farm data input,
 *  storage, summary, and export.
 *  
 *  Compile : javac FarmDataApp.java
 *  Run     : java FarmDataApp
 * ============================================================
 */

import java.util.*;
import java.io.*;
import java.time.*;
import java.time.format.*;
import java.util.stream.*;

// ── Domain model ─────────────────────────────────────────────
class CropRecord {
    private static int counter = 1;

    final int    id;
    final String date;
    String cropName;
    String fieldName;
    double areHectares;
    String soilType;
    double irrigationMm;
    double fertilizerKg;
    String notes;

    CropRecord(String cropName, String fieldName, double areaHa,
               String soilType, double irrigationMm, double fertilizerKg,
               String notes) {
        this.id            = counter++;
        this.date          = LocalDate.now().format(DateTimeFormatter.ofPattern("dd-MMM-yyyy"));
        this.cropName      = cropName;
        this.fieldName     = fieldName;
        this.areHectares   = areaHa;
        this.soilType      = soilType;
        this.irrigationMm  = irrigationMm;
        this.fertilizerKg  = fertilizerKg;
        this.notes         = notes;
    }

    // Total irrigation volume in m³
    double irrigationVolume() {
        return (irrigationMm / 1000.0) * areHectares * 10_000;
    }

    @Override
    public String toString() {
        return String.format(
            "  #%-3d | %-12s | %-10s | %5.2f ha | %-8s | %5.1f mm | %6.1f kg",
            id, cropName, fieldName, areHectares, soilType, irrigationMm, fertilizerKg
        );
    }
}

// ── Console colour helpers ────────────────────────────────────
class C {
    static final String RESET   = "\033[0m";
    static final String BOLD    = "\033[1m";
    static final String GREEN   = "\033[32m";
    static final String CYAN    = "\033[36m";
    static final String YELLOW  = "\033[33m";
    static final String RED     = "\033[31m";
    static final String MAGENTA = "\033[35m";
    static final String BLUE    = "\033[34m";
}

// ── Main application ─────────────────────────────────────────
public class FarmDataApp {

    private final List<CropRecord> records = new ArrayList<>();
    private final Scanner sc = new Scanner(System.in);

    // ── Entry point ──────────────────────────────────────────
    public static void main(String[] args) {
        new FarmDataApp().run();
    }

    void run() {
        printBanner();
        boolean active = true;
        while (active) {
            printMenu();
            int choice = readInt("Select option", 0, 6);
            switch (choice) {
                case 1 -> addRecord();
                case 2 -> viewAllRecords();
                case 3 -> searchRecords();
                case 4 -> showSummary();
                case 5 -> editRecord();
                case 6 -> exportToCSV();
                case 0 -> active = false;
            }
        }
        System.out.println(C.GREEN + C.BOLD +
            "\n  🌾  Farm data session ended. Goodbye!\n" + C.RESET);
    }

    // ── Banner ───────────────────────────────────────────────
    void printBanner() {
        System.out.println();
        divider('═', 60);
        System.out.println(C.BOLD + C.GREEN +
            "      🚜  FARM DATA INPUT & MANAGEMENT SYSTEM" + C.RESET);
        divider('═', 60);
        System.out.println("  Track crops, irrigation, soil, and fertilizer data.");
        divider('-', 60);
    }

    // ── Menu ─────────────────────────────────────────────────
    void printMenu() {
        System.out.println("\n" + C.BOLD + "  MAIN MENU" + C.RESET);
        System.out.println("  [1]  Add New Crop Record");
        System.out.println("  [2]  View All Records");
        System.out.println("  [3]  Search Records");
        System.out.println("  [4]  Show Farm Summary");
        System.out.println("  [5]  Edit a Record");
        System.out.println("  [6]  Export to CSV");
        System.out.println("  [0]  Exit");
        divider('-', 60);
    }

    // ── 1. Add Record ─────────────────────────────────────────
    void addRecord() {
        System.out.println("\n" + C.CYAN + C.BOLD + "  📋  ADD NEW CROP RECORD" + C.RESET);
        divider('-', 40);

        String crop  = readString("Crop name (e.g. Wheat, Rice, Maize)");
        String field = readString("Field / Plot name");
        double area  = readDouble("Area (hectares)", 0.01, 10000);

        System.out.println("  Soil types: [1] Sandy  [2] Loam  [3] Clay  [4] Silty");
        String[] soils = {"Sandy", "Loam", "Clay", "Silty"};
        int si = readInt("Select soil type", 1, 4);
        String soil = soils[si - 1];

        double irrigation  = readDouble("Irrigation applied (mm)", 0, 500);
        double fertilizer  = readDouble("Fertilizer applied (kg/ha)", 0, 1000);
        String notes       = readString("Notes (press ENTER to skip)", true);

        CropRecord r = new CropRecord(crop, field, area, soil, irrigation, fertilizer, notes);
        records.add(r);

        System.out.println(C.GREEN + "\n  ✅  Record #" + r.id + " added successfully!" + C.RESET);
        System.out.printf("      Irrigation volume = %.1f m³%n", r.irrigationVolume());
    }

    // ── 2. View All Records ───────────────────────────────────
    void viewAllRecords() {
        System.out.println("\n" + C.CYAN + C.BOLD + "  📄  ALL RECORDS (" + records.size() + ")" + C.RESET);
        if (records.isEmpty()) { noDataMsg(); return; }
        divider('-', 70);
        System.out.println("  #ID  | Crop         | Field      | Area     | Soil     | Irrig.   | Fertiliz.");
        divider('-', 70);
        records.forEach(r -> System.out.println(r));
        divider('-', 70);
    }

    // ── 3. Search ─────────────────────────────────────────────
    void searchRecords() {
        System.out.println("\n" + C.CYAN + C.BOLD + "  🔍  SEARCH RECORDS" + C.RESET);
        if (records.isEmpty()) { noDataMsg(); return; }
        String query = readString("Search by crop or field name").toLowerCase();
        List<CropRecord> found = records.stream()
            .filter(r -> r.cropName.toLowerCase().contains(query)
                      || r.fieldName.toLowerCase().contains(query))
            .collect(Collectors.toList());
        if (found.isEmpty()) {
            System.out.println(C.YELLOW + "  No records matched '" + query + "'." + C.RESET);
        } else {
            System.out.println("  Found " + C.GREEN + found.size() + C.RESET + " record(s):");
            divider('-', 70);
            found.forEach(r -> System.out.println(r));
        }
    }

    // ── 4. Summary ────────────────────────────────────────────
    void showSummary() {
        System.out.println("\n" + C.CYAN + C.BOLD + "  📊  FARM SUMMARY" + C.RESET);
        if (records.isEmpty()) { noDataMsg(); return; }
        divider('═', 50);

        double totalArea    = records.stream().mapToDouble(r -> r.areHectares).sum();
        double totalIrrigVol = records.stream().mapToDouble(CropRecord::irrigationVolume).sum();
        double avgIrrig     = records.stream().mapToDouble(r -> r.irrigationMm).average().orElse(0);
        double totalFert    = records.stream().mapToDouble(r -> r.fertilizerKg * r.areHectares).sum();

        Map<String, Long> cropCounts = records.stream()
            .collect(Collectors.groupingBy(r -> r.cropName, Collectors.counting()));

        System.out.printf("  %-28s : %d%n",     "Total Records",          records.size());
        System.out.printf("  %-28s : %.2f ha%n", "Total Area",             totalArea);
        System.out.printf("  %-28s : %.1f m³%n", "Total Irrigation Volume", totalIrrigVol);
        System.out.printf("  %-28s : %.1f mm%n", "Average Irrigation Depth", avgIrrig);
        System.out.printf("  %-28s : %.1f kg%n", "Total Fertilizer Used",   totalFert);

        System.out.println("\n  " + C.BOLD + "Crop Breakdown:" + C.RESET);
        cropCounts.forEach((crop, count) ->
            System.out.printf("    %-20s : %d field(s)%n", crop, count));

        // Highest irrigation field
        records.stream()
            .max(Comparator.comparingDouble(r -> r.irrigationMm))
            .ifPresent(r -> System.out.println("\n  " + C.YELLOW + "⚠  Highest irrigation: "
                + r.fieldName + " (" + r.irrigationMm + " mm)" + C.RESET));

        divider('═', 50);
    }

    // ── 5. Edit Record ────────────────────────────────────────
    void editRecord() {
        System.out.println("\n" + C.CYAN + C.BOLD + "  ✏️  EDIT RECORD" + C.RESET);
        if (records.isEmpty()) { noDataMsg(); return; }
        viewAllRecords();
        int id = readInt("Enter record ID to edit", 1, CropRecord.counter - 1);
        Optional<CropRecord> opt = records.stream().filter(r -> r.id == id).findFirst();
        if (opt.isEmpty()) {
            System.out.println(C.RED + "  Record #" + id + " not found." + C.RESET);
            return;
        }
        CropRecord r = opt.get();
        System.out.println("  Editing: " + r.cropName + " / " + r.fieldName);
        System.out.println("  [ENTER to keep current value]\n");

        String newCrop  = readOptional("New crop name [" + r.cropName + "]");
        String newField = readOptional("New field name [" + r.fieldName + "]");
        String newIrrig = readOptional("New irrigation mm [" + r.irrigationMm + "]");
        String newFert  = readOptional("New fertilizer kg/ha [" + r.fertilizerKg + "]");
        String newNotes = readOptional("New notes [" + r.notes + "]");

        if (!newCrop.isBlank())  r.cropName     = newCrop;
        if (!newField.isBlank()) r.fieldName    = newField;
        if (!newIrrig.isBlank()) r.irrigationMm = Double.parseDouble(newIrrig);
        if (!newFert.isBlank())  r.fertilizerKg = Double.parseDouble(newFert);
        if (!newNotes.isBlank()) r.notes        = newNotes;

        System.out.println(C.GREEN + "  ✅  Record #" + r.id + " updated." + C.RESET);
    }

    // ── 6. Export CSV ─────────────────────────────────────────
    void exportToCSV() {
        if (records.isEmpty()) { noDataMsg(); return; }
        String filename = "farm_data_" +
            LocalDate.now().format(DateTimeFormatter.ofPattern("yyyyMMdd")) + ".csv";
        try (PrintWriter pw = new PrintWriter(new FileWriter(filename))) {
            pw.println("ID,Date,Crop,Field,Area_ha,Soil,Irrigation_mm,IrrigVol_m3,Fertilizer_kg_ha,Notes");
            for (CropRecord r : records) {
                pw.printf("%d,%s,%s,%s,%.2f,%s,%.1f,%.1f,%.1f,%s%n",
                    r.id, r.date, r.cropName, r.fieldName,
                    r.areHectares, r.soilType,
                    r.irrigationMm, r.irrigationVolume(),
                    r.fertilizerKg, r.notes.replace(",", ";"));
            }
            System.out.println(C.GREEN + "  ✅  Exported " + records.size()
                + " records to: " + C.BOLD + filename + C.RESET);
        } catch (IOException e) {
            System.out.println(C.RED + "  ❌  Export failed: " + e.getMessage() + C.RESET);
        }
    }

    // ── Input helpers ─────────────────────────────────────────
    String readString(String prompt) {
        return readString(prompt, false);
    }

    String readString(String prompt, boolean optional) {
        while (true) {
            System.out.print(C.CYAN + "  → " + prompt + ": " + C.RESET);
            String val = sc.nextLine().trim();
            if (!val.isEmpty() || optional) return val;
            System.out.println(C.YELLOW + "    ⚠  Value required." + C.RESET);
        }
    }

    String readOptional(String prompt) {
        System.out.print(C.CYAN + "  → " + prompt + ": " + C.RESET);
        return sc.nextLine().trim();
    }

    double readDouble(String prompt, double min, double max) {
        while (true) {
            System.out.print(C.CYAN + "  → " + prompt + " [" + min + " – " + max + "]: " + C.RESET);
            try {
                double v = Double.parseDouble(sc.nextLine().trim());
                if (v >= min && v <= max) return v;
                System.out.println(C.YELLOW + "    ⚠  Enter a value between " + min + " and " + max + C.RESET);
            } catch (NumberFormatException e) {
                System.out.println(C.YELLOW + "    ⚠  Please enter a valid number." + C.RESET);
            }
        }
    }

    int readInt(String prompt, int min, int max) {
        while (true) {
            System.out.print(C.CYAN + "  → " + prompt + " [" + min + "–" + max + "]: " + C.RESET);
            try {
                int v = Integer.parseInt(sc.nextLine().trim());
                if (v >= min && v <= max) return v;
                System.out.println(C.YELLOW + "    ⚠  Enter a number between " + min + " and " + max + C.RESET);
            } catch (NumberFormatException e) {
                System.out.println(C.YELLOW + "    ⚠  Please enter a valid number." + C.RESET);
            }
        }
    }

    void noDataMsg() {
        System.out.println(C.YELLOW + "  No records yet. Add some using option [1]." + C.RESET);
    }

    static void divider(char ch, int len) {
        System.out.println("  " + String.valueOf(ch).repeat(len));
    }
}
