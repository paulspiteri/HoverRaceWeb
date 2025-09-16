import { createBrowserRouter, RouterProvider } from "react-router-dom";
import { Root } from "@/Root.tsx";
import { NoGame } from "@/NoGame.tsx";
import { GamePage } from "@/GamePage.tsx";

const router = createBrowserRouter([
    {
        path: "/",
        element: <Root />,
        children: [
            {
                index: true,
                element: <NoGame />,
            },
            {
                path: "game/:gameId",
                element: <GamePage />,
            },
        ],
    },
]);

function App() {
    return <RouterProvider router={router} />;
}

export default App;
