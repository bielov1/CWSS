import matplotlib.pyplot as plt

def plot_process_times(filename):
    indices = []
    finish_times = []

    with open(filename, 'r') as f:
        for idx, line in enumerate(f):
            line = line.strip()
            if not line:
                continue

            parts = line.split()

            time_str = parts[2].split(':')[1]
            finish_time = int(time_str)

            indices.append(idx)
            finish_times.append(finish_time)


    plt.plot(indices, finish_times, marker='o', linestyle='-')

    plt.xticks(range(0, len(indices), 50))

    plt.title("FLOOK 200 процесів")
    plt.xlabel("Номер запиту")
    plt.ylabel("Час виконання запиту us")
    plt.grid(True)
    plt.show()


if __name__ == "__main__":
    filename = "/home/oda/Programming/oda/CWSS/results/flook_200_res.txt"
    plot_process_times(filename)
