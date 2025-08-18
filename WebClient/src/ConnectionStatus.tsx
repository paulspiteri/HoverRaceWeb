export const ConnectionStatus: React.FC<{
  connectionId: string | undefined;
}> = ({ connectionId }) => {
  return (
    <div>
      <div>Connection ID: {connectionId ?? 'Connecting...'}</div>
    </div>
  );
};
