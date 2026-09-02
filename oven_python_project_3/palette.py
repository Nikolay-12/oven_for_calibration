import matplotlib
import matplotlib.pyplot as plt
from matplotlib.figure import Figure

COLORS1 = ["#940205", "#9c5207", "#a59a02", "#427d2b", "#01703e", "#007273", "#034076", "#091d66", "#3d0661",
           "#69015e"]
COLORS1_LABELS = ["красный", "оранжевый", "оранжево-желтый", "желтый", "желто-зеленый", "зеленый", "сине-зеленый",
                  "синий", "сине-фиолетовый", "фиолетовый"]

MAIN_COLORS = ["#fe020f", "#ff7f07", "#feff0b", "#82fd0e", "#03ff07", "#03fe8a", "#00fefe", "#0181fc", "#0002ff",
               "#7e06fd", "#dd00e3", "#ff0489"]
MAIN_COLORS_LABELS = ["красный", "оранжевый", "желтый", "желто-зеленый", "зеленый", "зелено-голубой", "голубой",
                      "лазурный", "синий", "сине-фиолетовый", "розовый", "розово-красный"]

LIGHT_COLORS = ["#fe7d82", "#ffbd83", "#ffff7d", "#bdff83", "#78fe83", "#7bffc1", "#79ffff", "#78bfff", "#767dff",
                "#bc7afe", "#fd7dff", "#fe7ac1"]
LIGHT_COLORS_LABELS = ["красный", "оранжевый", "желтый", "желто-зеленый", "зеленый", "зелено-голубой", "голубой",
                       "лазурный", "синий", "сине-фиолетовый", "розовый", "розово-красный"]

DARK_COLORS = ["#b00000", "#b05900", "#b3b200", "#5db103", "#02b001", "#01b25a", "#01b2b8", "#0259b6", "#0100b5",
               "#5b00b5", "#b301b6", "#b1015c"]
DARK_COLORS_LABELS = ["красный", "оранжевый", "желтый", "желто-зеленый", "зеленый", "зелено-голубой", "голубой",
                      "лазурный", "синий", "сине-фиолетовый", "розовый", "розово-красный"]

MAIN_COLORS_WITH_DARK_COLORS = ["#fe020f", "#ff7f07", "#feff0b", "#82fd0e", "#03ff07", "#03fe8a", "#00fefe", "#0181fc",
                                "#0002ff", "#7e06fd", "#dd00e3", "#ff0489", "#b00000", "#b05900", "#b3b200", "#5db103",
                                "#02b001", "#01b25a", "#01b2b8", "#0259b6", "#0100b5", "#5b00b5", "#69015e", "#b301b6",
                                "#b1015c"]

class PaletteInMatplotlib:
    @staticmethod
    def palette_in_matplotlib():
        #fig, ax = plt.subplots()
        fig = plt.figure(figsize=(10, 5), dpi=100, layout="tight")
        ax = fig.add_subplot(1, 1, 1)
        ax.set_title("Цветовая палитра", fontsize=8, fontweight='bold')
        ax.set_xlim(-0.1, 1.5)
        ax.set_ylim(0, 14)
        for i in range(10):
            plt.scatter(0, i+1, c=COLORS1[i])
            matplotlib.pyplot.annotate(COLORS1_LABELS[i], (0.01, i + 1))
            matplotlib.pyplot.annotate("палитра COLORS1", (0.0, 11))
        for i in range(12):
            plt.scatter(0.4, i + 1, c=LIGHT_COLORS[i])
            matplotlib.pyplot.annotate(LIGHT_COLORS_LABELS[i], (0.41, i + 1))
            matplotlib.pyplot.annotate("палитра LIGHT_COLORS", (0.4, 13))
            plt.scatter(0.8, i + 1, c=MAIN_COLORS[i])
            matplotlib.pyplot.annotate(MAIN_COLORS_LABELS[i], (0.81, i + 1))
            matplotlib.pyplot.annotate("палитра MAIN_COLORS", (0.8, 13))
            plt.scatter(1.2, i + 1, c=DARK_COLORS[i])
            matplotlib.pyplot.annotate(DARK_COLORS_LABELS[i], (1.21, i + 1))
            matplotlib.pyplot.annotate("палитра DARK_COLORS", (1.2, 13))
        plt.show()
