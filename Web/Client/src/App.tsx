import { createBrowserRouter, RouterProvider } from "react-router-dom";
import { Root } from "@/Root.tsx";
import { NoGame } from "@/NoGame.tsx";
import { GamePage } from "@/GamePage.tsx";
import { TrackRecordsPage } from "@/TrackRecordsPage.tsx";
import { About } from "@/About.tsx";
import { ErrorBoundary } from "@/ErrorBoundary.tsx";

const router = createBrowserRouter([
    {
        path: "/",
        element: <Root />,
        errorElement: <ErrorBoundary />,
        children: [
            {
                index: true,
                element: <NoGame />,
            },
            {
                path: "game/:gameId",
                element: <GamePage />,
            },
            {
                path: "records/:trackname",
                element: <TrackRecordsPage />,
            },
            {
                path: "about",
                element: <About />,
            },
        ],
    },
]);

function App() {
    return <RouterProvider router={router} />;
}

export default App;
