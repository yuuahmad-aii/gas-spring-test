import matplotlib.pyplot as plt
import csv


def visualize_data(file_name):
    # Membaca data dari file hasil.csv
    cycles = []
    beban_paling_negatif = []

    with open(file_name, "r") as file:
        reader = csv.DictReader(file)
        for row in reader:
            cycles.append(int(row["cycle"]))
            beban_paling_negatif.append(float(row["beban_paling_negatif"]))

    # Membuat plot
    plt.figure(figsize=(10, 6))
    plt.plot(
        cycles,
        beban_paling_negatif,
        marker="o",
        linestyle="-",
        color="b",
        label="Beban Paling Negatif",
    )

    # Menambahkan judul dan label
    plt.title("Visualisasi Beban Paling Negatif per Cycle", fontsize=14)
    plt.xlabel("Cycle", fontsize=12)
    plt.ylabel("Beban Paling Negatif", fontsize=12)
    plt.grid(True, linestyle="--", alpha=0.7)
    plt.legend()

    # Menampilkan plot
    plt.tight_layout()
    plt.show()


# Menjalankan fungsi
file_name = "hasil.csv"
visualize_data(file_name)
