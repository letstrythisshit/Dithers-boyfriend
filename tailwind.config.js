/** @type {import('tailwindcss').Config} */
export default {
  content: [
    "./index.html",
    "./src/**/*.{js,ts,jsx,tsx}",
  ],
  theme: {
    extend: {
      colors: {
        'app-bg': '#1e1e1e',
        'panel-bg': '#252525',
        'panel-border': '#3e3e3e',
        'input-bg': '#2d2d2d',
        'button-hover': '#3e3e3e',
        'accent': '#0e639c',
        'accent-hover': '#1177bb',
      },
    },
  },
  plugins: [],
}
