const temp = document.getElementById("temp");
const umid = document.getElementById("umid");
const gas = document.getElementById("gas");
const gas_amb = document.getElementById("gas_amb");
const statusHeader = document.getElementById("status-header");
const headerStatus = document.getElementById("header-status");
const connectBtn = document.getElementById("connect");
const toast = document.getElementById("toast");
const alertBanner = document.getElementById("alert-banner");
let port = null;
let isOpen = false;
let alertSound = null;

// Função para normalizar strings (remover acentos e converter para minúsculas)
function normalizeString(str) {
  return str
    .toLowerCase()
    .normalize("NFD")
    .replace(/[\u0300-\u036f]/g, "");
}

// Função para gerenciar o estado de alerta visual
function handleAlertStatus(status) {
  const allCards = document.querySelectorAll(".card");

  // Reset all cards
  allCards.forEach((card) => card.classList.remove("alarm"));

  // Normaliza o status para comparação
  const normalizedStatus = normalizeString(status);

  if (
    normalizedStatus.includes("gas perigo") ||
    normalizedStatus.includes("perigo")
  ) {
    // Mostrar banner de alerta
    alertBanner.style.display = "block";
    alertBanner.textContent = `⚠️ ALERTA: ${status} ⚠️`;
    alertBanner.style.background = "rgba(220, 53, 69, 0.9)";

    // Colocar cards relevantes em modo de alarme
    if (normalizedStatus.includes("gas")) {
      document.getElementById("card-gas").classList.add("alarm");
      document.getElementById("card-gas-amb").classList.add("alarm");
    } else {
      document.getElementById("card-temp").classList.add("alarm");
      document.getElementById("card-umid").classList.add("alarm");
    }

    // Tocar som de alerta
    playAlertSound();
  } else if (
    normalizedStatus.includes("atencao") ||
    normalizedStatus.includes("atencão") ||
    normalizedStatus.includes("atenção")
  ) {
    alertBanner.style.display = "block";
    alertBanner.textContent = `⚠️ ATENÇÃO: Condições fora do padrão ⚠️`;
    alertBanner.style.background = "rgba(255, 193, 7, 0.9)";
  } else if (
    normalizedStatus.includes("saudavel") ||
    normalizedStatus.includes("saudável")
  ) {
    // Esconder banner quando normal
    alertBanner.style.display = "none";
  } else {
    // Caso padrão para outros status desconhecidos
    alertBanner.style.display = "none";
  }
}

// Função para tocar som de alerta
function playAlertSound() {
  if (!alertSound) {
    // Som de beep em base64 para evitar dependência de arquivo externo
    alertSound = new Audio(
      "data:audio/wav;base64,UklGRigAAABXQVZFZm10IBAAAAABAAEARKwAAIhYAQACABAAZGF0YQQAAAAZGRkZ//8="
    );
    alertSound.volume = 0.5;
  }

  if (alertSound.paused) {
    alertSound
      .play()
      .catch((e) =>
        console.log("Não foi possível tocar o alerta de áudio:", e)
      );
  }
}

const chartData = {
  temperatura: [],
  umidade: [],
};

let optionsTemperatura = {
  series: [
    {
      name: "Temperatura",
      data: chartData.temperatura,
    },
  ],
  chart: {
    type: "line",
    height: 250,
    animations: {
      enabled: true,
      easing: "linear",
      dynamicAnimation: {
        speed: 1000,
      },
    },
    toolbar: {
      show: false,
    },
    zoom: {
      enabled: false,
    },
    background: "transparent",
    foreColor: "#d1d5db",
  },
  dataLabels: {
    enabled: false,
  },
  stroke: {
    curve: "smooth",
    width: 3,
    colors: ["#3498db"],
  },
  markers: {
    size: 4,
    colors: ["#2980b9"],
    strokeWidth: 0,
  },
  xaxis: {
    type: "datetime",
    labels: {
      datetimeUTC: false,
      style: {
        colors: "#d1d5db",
      },
    },
    axisBorder: {
      color: "rgba(255, 255, 255, 0.1)",
    },
    axisTicks: {
      color: "rgba(255, 255, 255, 0.1)",
    },
  },
  yaxis: {
    min: function (min) {
      return min - 2;
    },
    max: function (max) {
      return max + 2;
    },
    title: {
      text: "°C",
      style: {
        color: "#d1d5db",
      },
    },
    labels: {
      style: {
        colors: "#d1d5db",
      },
    },
  },
  tooltip: {
    x: {
      format: "HH:mm:ss",
    },
    theme: "dark",
  },
  grid: {
    borderColor: "rgba(255, 255, 255, 0.05)",
    strokeDashArray: 5,
  },
  fill: {
    type: "gradient",
    gradient: {
      shade: "dark",
      type: "vertical",
      shadeIntensity: 0.3,
      opacityFrom: 0.7,
      opacityTo: 0.2,
    },
  },
};

let optionsUmidade = {
  series: [
    {
      name: "Umidade",
      data: chartData.umidade,
    },
  ],
  chart: {
    type: "line",
    height: 250,
    animations: {
      enabled: true,
      easing: "linear",
      dynamicAnimation: {
        speed: 1000,
      },
    },
    toolbar: {
      show: false,
    },
    zoom: {
      enabled: false,
    },
    background: "transparent",
    foreColor: "#d1d5db",
  },
  dataLabels: {
    enabled: false,
  },
  stroke: {
    curve: "smooth",
    width: 3,
    colors: ["#1abc9c"],
  },
  markers: {
    size: 4,
    colors: ["#16a085"],
    strokeWidth: 0,
  },
  xaxis: {
    type: "datetime",
    labels: {
      datetimeUTC: false,
      style: {
        colors: "#d1d5db",
      },
    },
    axisBorder: {
      color: "rgba(255, 255, 255, 0.1)",
    },
    axisTicks: {
      color: "rgba(255, 255, 255, 0.1)",
    },
  },
  yaxis: {
    min: function (min) {
      return min - 5;
    },
    max: function (max) {
      return max + 5;
    },
    title: {
      text: "%",
      style: {
        color: "#d1d5db",
      },
    },
    labels: {
      style: {
        colors: "#d1d5db",
      },
    },
  },
  tooltip: {
    x: {
      format: "HH:mm:ss",
    },
    theme: "dark",
  },
  grid: {
    borderColor: "rgba(255, 255, 255, 0.05)",
    strokeDashArray: 5,
  },
  fill: {
    type: "gradient",
    gradient: {
      shade: "dark",
      type: "vertical",
      shadeIntensity: 0.3,
      opacityFrom: 0.7,
      opacityTo: 0.2,
    },
  },
};

document.addEventListener("DOMContentLoaded", function () {
  window.chartTemperatura = new ApexCharts(
    document.querySelector("#chart-temperatura"),
    optionsTemperatura
  );
  window.chartUmidade = new ApexCharts(
    document.querySelector("#chart-umidade"),
    optionsUmidade
  );

  window.chartTemperatura.render();
  window.chartUmidade.render();

  // Inicializar banner de alerta como escondido
  alertBanner.style.display = "none";
});

function showToast(msg, type = "info") {
  let icon = "";
  if (type === "success") {
    icon = `<svg class="icon" width="20" height="20" fill="#27ae60" viewBox="0 0 20 20"><circle cx="10" cy="10" r="10" fill="rgba(39, 174, 96, 0.2)"/><path d="M6 10.5l2.5 2.5 5-5" stroke="#27ae60" stroke-width="2" fill="none"/></svg>`;
  } else if (type === "error") {
    icon = `<svg class="icon" width="20" height="20" fill="#e74c3c" viewBox="0 0 20 20"><circle cx="10" cy="10" r="10" fill="rgba(231, 76, 60, 0.2)"/><path d="M7 7l6 6M13 7l-6 6" stroke="#e74c3c" stroke-width="2" fill="none"/></svg>`;
  } else {
    icon = `<svg class="icon" width="20" height="20" fill="#3498db" viewBox="0 0 20 20"><circle cx="10" cy="10" r="10" fill="rgba(52, 152, 219, 0.2)"/><text x="10" y="15" text-anchor="middle" font-size="12" fill="#3498db">i</text></svg>`;
  }
  toast.innerHTML = icon + `<span>${msg}</span>`;
  toast.className = `show ${type}`;
  setTimeout(() => {
    toast.className = toast.className.replace("show", "");
  }, 3500);
}

connectBtn.onclick = async () => {
  if (!("serial" in navigator)) {
    showToast(
      "Seu navegador não suporta Web Serial API. Use Chrome ou Edge.",
      "error"
    );
    return;
  }
  if (isOpen) {
    showToast("A porta já está aberta!", "info");
    return;
  }
  try {
    port = await navigator.serial.requestPort();
    await port.open({ baudRate: 115200 });
    isOpen = true;
    showToast("Conectado com sucesso!", "success");
    connectBtn.textContent = "Sensor Conectado";
    connectBtn.style.background = "var(--success)";
  } catch (err) {
    showToast("Erro ao abrir a porta serial: " + err.message, "error");
    return;
  }

  const decoder = new TextDecoderStream();
  port.readable.pipeTo(decoder.writable);
  const reader = decoder.readable.getReader();

  let buffer = "";
  while (true) {
    try {
      const { value, done } = await reader.read();
      if (done) break;
      buffer += value;
      let lines = buffer.split("\n");
      buffer = lines.pop();
      for (let line of lines) {
        try {
          if (line.includes('"gas":') && !line.includes('"gas":"')) {
            line = line
              .replace(/"gas":([^,}]+)/g, '"gas":"$1"')
              .replace(/"gas_amb":([^,}]+)/g, '"gas_amb":"$1"');
          }

          const data = JSON.parse(line);

          temp.textContent = data.temperatura;
          umid.textContent = data.umidade;
          gas.textContent = data.gas;
          gas_amb.textContent = data.gas_amb;

          const timestamp = new Date().getTime();

          if (chartData.temperatura.length > 50) {
            chartData.temperatura.shift();
            chartData.umidade.shift();
          }

          chartData.temperatura.push([timestamp, data.temperatura]);
          chartData.umidade.push([timestamp, data.umidade]);

          if (window.chartTemperatura && window.chartUmidade) {
            window.chartTemperatura.updateSeries([
              {
                data: chartData.temperatura,
              },
            ]);
            window.chartUmidade.updateSeries([
              {
                data: chartData.umidade,
              },
            ]);
          }

          statusHeader.textContent = data.status;
          headerStatus.classList.remove("saude", "atencao", "perigo");

          // Chamar a função de alerta para atualizar o UI
          handleAlertStatus(data.status);

          // Normaliza o status para comparação mais robusta
          const normalizedStatus = normalizeString(data.status);

          let statusIcon = "";
          if (normalizedStatus.includes("perigo")) {
            headerStatus.classList.add("perigo");
            statusIcon = `<svg class="icon" width="20" height="20" fill="var(--danger)" viewBox="0 0 20 20"><circle cx="10" cy="10" r="10" fill="rgba(231, 76, 60, 0.2)"/><path d="M7 7l6 6M13 7l-6 6" stroke="var(--danger)" stroke-width="2" fill="none"/></svg>`;
          } else if (
            normalizedStatus.includes("atencao") ||
            normalizedStatus.includes("atenção")
          ) {
            headerStatus.classList.add("atencao");
            statusIcon = `<svg class="icon" width="20" height="20" fill="var(--warning)" viewBox="0 0 20 20"><circle cx="10" cy="10" r="10" fill="rgba(246, 185, 59, 0.2)"/><text x="10" y="15" text-anchor="middle" font-size="12" fill="var(--warning)">!</text></svg>`;
          } else {
            headerStatus.classList.add("saude");
            statusIcon = `<svg class="icon" width="20" height="20" fill="var(--success)" viewBox="0 0 20 20"><circle cx="10" cy="10" r="10" fill="rgba(39, 174, 96, 0.2)"/><path d="M6 10.5l2.5 2.5 5-5" stroke="var(--success)" stroke-width="2" fill="none"/></svg>`;
          }
          headerStatus.innerHTML =
            statusIcon + `<span id="status-header">${data.status}</span>`;
        } catch (e) {
          console.error("Erro ao processar dados:", e, line);
        }
      }
    } catch (err) {
      showToast("Erro de leitura da serial: " + err.message, "error");
      connectBtn.textContent = "Conectar Sensor";
      connectBtn.style.background = "var(--accent)";

      // Resetar alertas quando a conexão cair
      alertBanner.style.display = "none";
      document
        .querySelectorAll(".card")
        .forEach((card) => card.classList.remove("alarm"));

      break;
    }
  }
  isOpen = false;
};
