# example_engine.py
import math

class GameEngine:
    """Processes user input and manages internal state."""

    def __init__(self):
        self.running = True


    def process_input(self, key, width=None, height=None):
        """
        Returns a tuple: (target, content)
        target: 'status' or 'main'
        content: string or list of strings
        """
        if key == ord('q'):
            self.running = False
            return ("status", "Quitting...")

        elif key == ord('h'):
            return ("status", "Help: Press 'q' to quit, 'f' for fractal demo, 'c' to clear the status bar, k to see custom colour demo.")

        elif key == ord('c'):
            return ("status", "")  # Clears the status bar

        elif key == ord('f'):
            return ("main", self.generate_ascii_fractal(width, height))
        elif key == ord('k'):
            # 'k' triggers a UI-only action (custom colour) handled by the UI
            return ("action", "demo_custom_color")
        else:
            return ("status", f"Pressed key: {chr(key) if key < 256 else key}")

    def generate_ascii_fractal(self, width, height):
        """Generate an ASCII pattern based on sine and cosine waves."""
        pattern = []
        for y in range(height):
            row = []
            for x in range(width):
                xf = x / width * 4 * math.pi
                yf = y / height * 4 * math.pi
                val = math.sin(xf) * math.cos(yf)
                if val > 0.7:
                    ch = "#"
                elif val > 0.3:
                    ch = "*"
                elif val > 0:
                    ch = "+"
                elif val > -0.3:
                    ch = "."
                else:
                    ch = " "
                row.append(ch)
            pattern.append("".join(row))
        return pattern
