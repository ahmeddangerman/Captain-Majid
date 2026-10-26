const buttons = Array.from(document.querySelectorAll(".stage-menu button"));
const title = document.getElementById("stage-title");
const copy = document.getElementById("stage-copy");
const currentStage = document.getElementById("stage-current");
const digits = ["١", "٢", "٣", "٤"];

function selectStage(index, moveFocus = false) {
  const selected = (index + buttons.length) % buttons.length;
  buttons.forEach((button, i) => {
    const active = i === selected;
    button.setAttribute("aria-pressed", String(active));
    if (active) {
      title.textContent = button.dataset.title;
      copy.textContent = button.dataset.copy;
      currentStage.textContent = digits[i];
      if (moveFocus) button.focus();
    }
  });
}

buttons.forEach((button, index) => {
  button.addEventListener("click", () => selectStage(index));
  button.addEventListener("keydown", event => {
    if (event.key === "ArrowDown" || event.key === "ArrowUp") {
      event.preventDefault();
      selectStage(index + (event.key === "ArrowDown" ? 1 : -1), true);
    } else if (event.key === "Home") {
      event.preventDefault();
      selectStage(0, true);
    } else if (event.key === "End") {
      event.preventDefault();
      selectStage(buttons.length - 1, true);
    }
  });
});

