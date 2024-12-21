import matplotlib.pyplot as plt

def parse_total_time_from_file(filename):
    with open(filename, 'r') as f:
        line = f.readline().strip()

        parts = line.split("time:")
        if len(parts) < 2:
            raise ValueError(f"Не вдалося знайти 'time:' у файлі {filename}")

        time_str = parts[1].strip()
        return int(time_str)

def plot_algorithms_comparison():

    algorithms = ["FIFO", "LOOK", "FLOOK"]
    

    files = {
        "FIFO" :  "/home/oda/Programming/oda/CWSS/results/fifo_total_time_50.txt",
        "LOOK" :  "/home/oda/Programming/oda/CWSS/results/look_total_time_50.txt",
        "FLOOK":  "/home/oda/Programming/oda/CWSS/results/flook_total_time_50.txt"
    }
    

    times = []
    
    for alg in algorithms:
        filename = files[alg]
        total_time = parse_total_time_from_file(filename)
        times.append(total_time)


    plt.figure(figsize=(8, 5))
    plt.bar(algorithms, times, color=['blue', 'orange', 'green'])
    plt.xlabel("Алгоритми планування")
    plt.ylabel("Загальний час виконання запитів us")
    plt.title("50 процесів")


    for i, t in enumerate(times):
        plt.text(i, t + 0.01 * t, str(t), ha='center', va='bottom')

    plt.grid(axis='y', linestyle='--', alpha=0.7)
    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    plot_algorithms_comparison()
