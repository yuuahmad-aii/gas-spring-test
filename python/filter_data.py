import csv


def find_most_negative_load(input_file, output_file):
    # Dictionary untuk menyimpan beban paling negatif per cycle
    cycle_min_load = {}

    # Membaca file input
    with open(input_file, "r") as file:
        reader = csv.DictReader(file)
        for row in reader:
            cycle = int(row["cycle"])
            beban = float(row["beban"])

            # Periksa apakah cycle sudah ada di dictionary
            if cycle not in cycle_min_load or beban < cycle_min_load[cycle]:
                cycle_min_load[cycle] = beban

    # Menulis hasil ke file output
    with open(output_file, "w", newline="") as file:
        writer = csv.writer(file)
        writer.writerow(["cycle", "beban_paling_negatif"])

        for cycle, beban in sorted(cycle_min_load.items()):
            writer.writerow([cycle, beban])

    print(f"Hasil telah disimpan di {output_file}")


# Menjalankan fungsi
input_file = "data_logger_2.txt"
output_file = "hasil.csv"
find_most_negative_load(input_file, output_file)
